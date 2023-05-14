#include "pe_exchange.h"

/** Signal handling - queue implementation */

typedef struct {
    int signum;
    siginfo_t *sinfo;
    void *context;
} queued_signal;

volatile queued_signal signal_queue[MAX_SIGNALS]; // array of queued signals
volatile sig_atomic_t queue_size = 0; // global number of queued sigs

void signal_handler_read(int s, siginfo_t* sinfo, void * context) {
    if (queue_size < MAX_SIGNALS) {
        signal_queue[queue_size].signum = s;
        signal_queue[queue_size].sinfo = sinfo;
        signal_queue[queue_size].context = context;
        
        queue_size++;
    }
}

void signal_handler_disc(int s, siginfo_t* sinfo, void * context) {
    if (queue_size < MAX_SIGNALS) {
        signal_queue[queue_size].signum = s;
        signal_queue[queue_size].sinfo = sinfo;
        signal_queue[queue_size].context = context;
        queue_size++;
    }
}

/** 
 * ORDER MATCHING
 * 1) retrieve head of product sell LL (cheapest sell price)
 * 2) retrieve tail of product buy orders (highest buy price)
 * 3) if buy_price >= sell_price, match order
 * 4) update position
 * 5) 
 */

void check_match(product * p, long long int * fees) {

    list_node* to_delete[MAX_DELETE] = {0}; // product list nodes
    int c = 0;

    list_node * sell_cursor = p->sell_orders;
    list_node * buy_cursor = p->buy_orders;

    order * sell;
    order * buy;

    position * seller_pos;
    position * buyer_pos;

    int quantity_sold;
    long long int value;

    while (sell_cursor != NULL && buy_cursor != NULL) {

        sell = sell_cursor->data.order;
        buy = buy_cursor->data.order;

        if (buy->unit_cost < sell->unit_cost) {
            break;
        }
            

        seller_pos = find_position(sell->broker, p);
        buyer_pos = find_position(buy->broker, p);

        quantity_sold = 0;
        value = 0;

        if (buy->quantity == sell->quantity) {

            // update position quantities
            quantity_sold = buy->quantity;
            // update position values

            to_delete[c] = sell_cursor;
            c++;
            sell_cursor = sell_cursor->next;

            to_delete[c] = buy_cursor;
            c++;
            buy_cursor = buy_cursor->next;
            
        } 

        else if (buy->quantity < sell->quantity) {

            // update positions
            quantity_sold = buy->quantity;

            sell->quantity -= quantity_sold;

            to_delete[c] = buy_cursor;
            c++;
            buy_cursor = buy_cursor->next;
        } 

        else if (buy->quantity > sell->quantity) {
            
            //update positions q
            quantity_sold = sell->quantity;
            //update pos value
            
            buy->quantity -= quantity_sold;

            to_delete[c] = sell_cursor;
            c++;
            sell_cursor = sell_cursor->next;
        }

        double fees_paid;
        double multiplier = FEE_PERCENTAGE * 0.01;

        if (sell->time < buy->time) {

            value = (long long int) quantity_sold * sell->unit_cost;
            fees_paid = round(value * multiplier);

            buyer_pos->value -= fees_paid;

            printf("%s Match: Order %i [T%i], New Order %i [T%i], value: $%lli, fee: $%lli.\n",
                LOG_PREFIX, sell->order_id, sell->broker->id, 
                    buy->order_id, buy->broker->id, value, (long long int)fees_paid);
        } else {
            value = quantity_sold * buy->unit_cost;
            fees_paid = round(value * multiplier);

            seller_pos->value -= fees_paid;

            printf("%s Match: Order %i [T%i], New Order %i [T%i], value: $%lli, fee: $%lli.\n",
                LOG_PREFIX, buy->order_id, buy->broker->id, 
                    sell->order_id, sell->broker->id, value, (long long int)fees_paid);
        }

        seller_pos->quantity -= quantity_sold;
        buyer_pos->quantity += quantity_sold;

        seller_pos->value += value;
        buyer_pos->value -= value;

        SEND_FILL(sell->broker, sell, quantity_sold);
        SEND_FILL(buy->broker, buy, quantity_sold);

        *fees += (long long int) fees_paid;

    }

    for (int i = 0; i < c; i++) {
        order * o = to_delete[i]->data.order;
        list_node * trader_node = find_order_listnode(o->broker, o->order_id);

        list_delete_node_only(&o->broker->orders, trader_node);

        if (o->type == BUY) {
            list_delete_recursive(&p->buy_orders, to_delete[i]);
        } else {
            list_delete_recursive(&p->sell_orders, to_delete[i]);
        }
    }
    return;
}

void parse_command(trader * t, char * command, list_node * product_head, trader * traders, int n, int time, long long int * fees) {
    printf("%s [T%d] Parsing command: <%s>\n", LOG_PREFIX, t->id, command);

    int order_id;
    int quantity;
    int unit_price;
    char product_name[PRODUCT_LEN] = {0};
    char word[CMD_LEN];

    if (id_command(command, word) == 0) {
        SEND_STATUS(t, -1, INVALID);
        return;
    }

    if (strcmp(word, "BUY") == 0) {
        int invalid_data = 0;

        if (sscanf(command, "BUY %i %s %i %i%n", &order_id, product_name, &quantity, &unit_price, &invalid_data) != 4) {
            SEND_STATUS(t, -1, INVALID);
			return;
        }

        if (command[invalid_data] != '\0') {
            SEND_STATUS(t, -1, INVALID);
			return;
        }

        if (order_id != t->next_order_id) {
            SEND_STATUS(t, -1, INVALID);
			return;
        }

        list_node * l = list_find(product_head, product_name);
        if (l == NULL) {
            SEND_STATUS(t, -1, INVALID);
			return;
        }

        product * p = l->data.product;

        if (quantity < 1 || quantity > 999999) {
            SEND_STATUS(t, -1, INVALID);
			return;
        }

        if (unit_price < 1 || unit_price > 999999) {
            SEND_STATUS(t, -1, INVALID);
			return;
        }

        /** order is valid
         * - setup order struct
         * - append order struct to product BUY + trader LL
         * - increment t.next_order for future validity check
         * - send ACCEPTED to trader; 
         * - send MARKET msg to all other trader
         */

        order * o = malloc(sizeof(order));

        o->broker = t;
        o->product = p;
        o->type = BUY;

        o->quantity = quantity;
        o->unit_cost = unit_price;
        o->order_id = order_id;
        o->time = time;

        // ADD TO LIST
        list_add(&t->orders, o, ORDER);
        p->buy_orders = list_add_sorted_desc(p->buy_orders, o, ORDER);

        t->next_order_id++;

        //SEND_ACCEPTED
        SEND_STATUS(t, order_id, ACCEPTED);

        //SEND_MARKET_UPDATE
        SEND_MARKET_UPDATE(traders, n, *o, t);
        
        //CHECK_MATCH_AND_FILL
        check_match(o->product, fees);

        //REPORTING
        spx_report(product_head, traders, n);

    } 

    else if (strcmp(word, "SELL") == 0) {
        int invalid_data = 0;

        if (sscanf(command, "SELL %i %s %i %i%n", &order_id, product_name, &quantity, &unit_price, &invalid_data) != 4) {
            SEND_STATUS(t, -1, INVALID);
			return;
        }

        if (command[invalid_data] != '\0') {
            SEND_STATUS(t, -1, INVALID);
			return;
        }

        if (order_id != t->next_order_id) {
            SEND_STATUS(t, -1, INVALID);
			return;
        }

        list_node * l = list_find(product_head, product_name);
        if (l == NULL) {
			SEND_STATUS(t, -1, INVALID);
			return;

        }
        product * p = (product *)l->data.product;

        if (quantity < 1 || quantity > 999999) {
            SEND_STATUS(t, -1, INVALID);
			return;
        }

        if (unit_price < 1 || unit_price > 999999) {
            SEND_STATUS(t, -1, INVALID);
			return;
        }

        order * o = malloc(sizeof(order));

        o->broker = t;
        o->product = p;
        o->type = SELL;

        o->quantity = quantity;
        o->unit_cost = unit_price;
        o->order_id = order_id;
        o->time = time;

        // ADD NEW ORDER TO LLs
        list_add(&t->orders, o, ORDER);
        p->sell_orders = list_add_sorted_asc(p->sell_orders, o, ORDER);
    
        t->next_order_id++;

        //SEND_ACCEPTED
        SEND_STATUS(t, o->order_id, ACCEPTED);

        //SEND_MARKET_UPDATE
        SEND_MARKET_UPDATE(traders, n, *o, t);
        //CHECK_MATCH_AND_FILL
        check_match(o->product, fees);

        //REPORTING
        spx_report(product_head, traders, n);
    }

    else if (strcmp(word, "AMEND") == 0) {
        int invalid_data = 0;

        if (sscanf(command, "AMEND %i %i %i%n", &order_id, &quantity, &unit_price, &invalid_data) != 3) {
			SEND_STATUS(t, -1, INVALID);
			return;
        }   

        if (command[invalid_data] != '\0') {
            SEND_STATUS(t, -1, INVALID);
			return;
        }

        if (quantity < 1 || quantity > 999999) {
            SEND_STATUS(t, -1, INVALID);
			return;
        }

        if (unit_price < 1 || unit_price > 999999) {
            SEND_STATUS(t, -1, INVALID);
			return;
        }

        order * to_amend = find_trader_order(t, order_id);
        if (to_amend == NULL) {
            SEND_STATUS(t, -1, INVALID);
            return;
        }

        /** Amend order */

        to_amend->time = time;
        to_amend->quantity = quantity;
        to_amend->unit_cost = unit_price;

        // update nodes - delete and then re-add to product LL

        list_node * to_amend_ll = find_product_order_listnode(to_amend->product, to_amend);

        if (to_amend->type == BUY) {
            list_delete_node_only(&to_amend->product->buy_orders, to_amend_ll);
            to_amend->product->buy_orders = list_add_sorted_desc(to_amend->product->buy_orders, to_amend, ORDER);
        } else {
            list_delete_node_only(&to_amend->product->sell_orders, to_amend_ll);
            to_amend->product->sell_orders = list_add_sorted_asc(to_amend->product->sell_orders, to_amend, ORDER);
        }

        //SEND_ACCEPTED //
        SEND_STATUS(t, order_id, AMENDED);

        //SEND_MARKET_UPDATE??
        SEND_MARKET_UPDATE(traders, n, *to_amend, t);
        //CHECK_MATCH_AND_FILL
        check_match(to_amend->product, fees);

        //REPORTING
        spx_report(product_head, traders, n);
    } 

    else if (strcmp(word, "CANCEL") == 0) {
        int invalid_data = 0;

        if (sscanf(command, "CANCEL %i%n", &order_id, &invalid_data) != 1) {
			SEND_STATUS(t, -1, INVALID);
			return;
        }

        if (command[invalid_data] != '\0') {
            SEND_STATUS(t, -1, INVALID);
			return;
        }

        list_node * to_cancel = find_order_listnode(t, order_id);
        if (to_cancel == NULL) {
            SEND_STATUS(t, -1, INVALID);
			return;
        }

        order * o = to_cancel->data.order;
        o->quantity = 0;
        o->unit_cost = 0;

        list_node * p_node_cancel = find_product_order_listnode(o->product, o);

        //SEND_CONFIRM
        SEND_STATUS(t, order_id, CANCELLED);
        //MARKET_UPDATE
        SEND_MARKET_UPDATE(traders, n, *o, t);

        // delete node only from trader
        list_delete_node_only(&o->broker->orders, to_cancel);

        // delete node and data from relevant product
        if (o->type == BUY) {
            list_delete_recursive(&o->product->buy_orders, p_node_cancel);
        } else {
            list_delete_recursive(&o->product->sell_orders, p_node_cancel);
        }
    
        spx_report(product_head, traders, n);

    } else {
        SEND_STATUS(t, -1, INVALID);
        return;
    }
}

/** Main
 * 
 * 
 */

int main(int argc, char const *argv[])
{

    if (argc < 2) {
        printf("Invalid launch options\n");
        return 2;
    }

    char buffer[BUFFER_LEN];
    
    long long int fees = 0;
    int time = 0;

    printf("%s Starting\n", LOG_PREFIX); 

    //init products and traders data structures
    list_node* products_ll = init_products(argv[1]);
    struct trader * traders = get_traders(argc, argv, products_ll);

    /* Signal handling */

    struct sigaction s_sigusr = {0};
    s_sigusr.sa_flags |= SA_SIGINFO;
    s_sigusr.sa_flags |= SA_RESTART;

    s_sigusr.sa_sigaction = signal_handler_read; // could block other signals at this point
    if (sigaction(SIGUSR1, &s_sigusr, NULL) == -1) {
        perror("Failed to bind signal_handler for SIGUSR1");
        exit(2);
    }

    struct sigaction s_sigchld = {0};
    s_sigchld.sa_flags |= SA_SIGINFO;
    s_sigchld.sa_flags |= SA_RESTART;

    s_sigchld.sa_sigaction = signal_handler_disc;
    if (sigaction(SIGCHLD, &s_sigchld, NULL) == -1) {
        perror("Failed to bind signal_handler for SIGCHLD");
        exit(2);
    }

    // create trader pipes, fork, fifo connects
    init_traders(traders, argc - 2); 
   
    SEND_MARKET_OPEN(traders, argc-2);

    sigset_t allset, oldset;
    sigfillset(&allset);
    sigprocmask(SIG_BLOCK, &allset, &oldset);

    // ignore sigpipe incase trader disc early
    signal(SIGPIPE, SIG_IGN);

    while(1) 
    {

        if (queue_size == 0 && number_of_live_traders(traders, argc-2) != 0) {
            sigprocmask(SIG_SETMASK, &oldset, NULL);
            pause();
            sigprocmask(SIG_BLOCK, &allset, NULL);  // Block all signals again
        }

        while (queue_size > 0) {
            queued_signal s = signal_queue[0];

            switch(s.signum) {
                case SIGUSR1:
                {
                    trader * sender = find_trader(s.sinfo->si_pid, traders, argc-2);
                    if (sender == NULL)
                        continue;
                    
                    receive_data(sender->incoming_fd, buffer);
                    parse_command(sender, buffer, products_ll, traders, argc-2, time, &fees);
                    
                    break;
                }
                
                case SIGCHLD:
                {
                    trader * sender = find_trader(s.sinfo->si_pid, traders, argc-2);
                    if (sender == NULL)
                        continue;

                    sender->online = 0;
                    printf("%s Trader %i disconnected\n", LOG_PREFIX, sender->id);

                    break;
                }
                
                default:
                    continue;
            }

             // bump queue
            for (int i = 0; i < queue_size - 1; i++) {
                signal_queue[i] = signal_queue[i + 1];
            }

            queue_size--;
        }

        time++;

        if (number_of_live_traders(traders, argc-2) == 0)
            break;

    }

   /* Endgame
    * - Print [PEX] Trader <Trader ID> disconnected
    * - Reject pending / concurrent orders, maintain existing
    * - [PEX] Trading completed
    * - [PEX] Exchange fees collected: $<total fees>
    */

    printf("%s Trading completed\n", LOG_PREFIX);
    printf("%s Exchange fees collected: $%lli\n", LOG_PREFIX, fees);

    /* Close fds */

    for (int i = 0; i < argc-2; i++) {
        close_fifos(&traders[i]);

        /** unlink fifos */

        char path_temp[PATH_LEN] = {0};
        snprintf(path_temp, PATH_LEN, FIFO_EXCHANGE, traders[i].id);
        unlink(path_temp);

        snprintf(path_temp, PATH_LEN, FIFO_TRADER, traders[i].id);
        unlink(path_temp);

        list_free_recursive(traders[i].orders);
        list_free_recursive(traders[i].positions);
    }

    /* Free memory */
    

    list_free_node(products_ll); 
    
    /** Free each trader position + order*/

    free(traders);

    exit(0);
}