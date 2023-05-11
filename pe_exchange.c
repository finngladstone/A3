#include "pe_exchange.h"

/** 
 * Open fd for products list 
 * Initialise LL for products using Richard's template
 * Output status message
 * Need to call list_free() upon endgame!
 */

//global

int trader_ready = -1;
int trader_disconnect = -1;

int time;

void signal_handler_read(int s, siginfo_t* sinfo, void * context) {
    if (s == SIGUSR1) {
        trader_ready = sinfo->si_pid;
    }
}

void signal_handler_disc(int s, siginfo_t* sinfo, void * context) {
    if (s == SIGCHLD) {
        trader_disconnect = sinfo->si_pid;
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

void check_match(product * p) {
    list_node * sell_cursor = p->sell_orders;
    list_node * buy_cursor = list_get_tail(p->buy_orders);

    order * sell;
    order * buy;

    position * seller_pos;
    position * buyer_pos;

    while (sell_cursor && buy_cursor) {

        sell = sell_cursor->data.order;
        buy = buy_cursor->data.order;

        if (buy->unit_cost < sell->unit_cost) 
            break;

        seller_pos = find_position(sell->broker, p);
        buyer_pos = find_position(buy->broker, p);

        int quantity_sold;
        int value;

        if (buy->quantity == sell->quantity) {

            // update position quantities
            quantity_sold = buy->quantity;

            // update position values
            value = buy->quantity * sell->unit_cost;

            //update cursors 
            sell_cursor = sell_cursor->next;
            buy_cursor = buy_cursor->prev;

            // delete fulfilled orders
            list_delete(&p->sell_orders, sell_cursor->prev);
            list_delete(&p->buy_orders, buy_cursor->prev);
            
        } 

        else if (buy->quantity < sell->quantity) {

            // update positions
            int quantity_sold = buy->quantity;

            value = quantity_sold * sell->unit_cost;
            
            sell->quantity -= quantity_sold;

            //update cursors - dont move next sold cursor -- GOING BACKWARDS IN BUY ORDERS
            buy_cursor = buy_cursor->prev;

            // delete fulfilled BUY ORDER
            list_delete(&p->buy_orders, buy_cursor->next);

        } 

        else if (buy->quantity > sell->quantity) {
            
            //update positions q
            quantity_sold = sell->quantity;
            

            //update pos value
            value = quantity_sold * sell->unit_cost;
            
            buy->quantity -= quantity_sold;

            //update cursor - dont move buy cursor as unfulfilled
            sell_cursor = sell_cursor->next;

            // delete fulfilled SELL ORDER
            list_delete(&p->sell_orders, sell_cursor->prev);
        }

        seller_pos->quantity -= quantity_sold;
        buyer_pos->quantity += quantity_sold;

        seller_pos->value += value;
        buyer_pos->value -= value;

        SEND_FILL(sell->broker, sell, quantity_sold);
        SEND_FILL(buy->broker, buy, quantity_sold);
        
    }

    return;
    
}

void parse_command(trader * t, char * command, list_node * product_head, trader * traders, int n) {
    // verbose
    printf("%s [T%d] Parsing command: <%s>\n", LOG_PREFIX, t->id, command);

    int order_id;
    int quantity;
    int unit_price;
    char product_name[PRODUCT_LEN] = {0};
    char word[CMD_LEN];

    if (id_command(command, word) == 0) {
        printf("%s Invalid command: <%s>\n", LOG_PREFIX, command);
        SEND_STATUS(t, -1, INVALID);
        return;
    }

    if (strcmp(word, "BUY") == 0) {
        if (sscanf(command, "BUY %i %s %i %i", &order_id, product_name, &quantity, &unit_price) != 4) {
			printf("%s Invalid command format: <%s>\n", LOG_PREFIX , command); 
            SEND_STATUS(t, -1, INVALID);
			return;
        }

        if (order_id != t->next_order_id) {
			printf("%s Order ID %i invalid\n", LOG_PREFIX, order_id);
            SEND_STATUS(t, -1, INVALID);
			return;
        }

        list_node * l = list_find(product_head, product_name);
        if (l == NULL) {
			printf("%s Invalid product name: <%s>\n", LOG_PREFIX ,product_name);
            SEND_STATUS(t, -1, INVALID);
			return;
        }

        product * p = l->data.product;

        if (quantity < 1 || quantity > 999999) {
			printf("%s Invalid quantity: <%i>\n", LOG_PREFIX, quantity);
            SEND_STATUS(t, -1, INVALID);
			return;
        }

        if (unit_price < 1 || unit_price > 999999) {
			printf("%s Invalid price <$%i>\n", LOG_PREFIX, unit_price);
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

        order o = {0};

        o.broker = t;
        o.product = p;
        o.type = BUY;

        o.quantity = quantity;
        o.unit_cost = unit_price;
        o.order_id = order_id;
        o.time = time;

        list_add(&t->orders, &o, ORDER);
        list_add_sorted(&p->buy_orders, &o, ORDER);

        t->next_order_id++;

        //SEND_ACCEPTED
        SEND_STATUS(t, order_id, ACCEPTED);

        //SEND_MARKET_UPDATE
        SEND_MARKET_UPDATE(traders, n, o, t);
        //CHECK_MATCH_AND_FILL

    } 

    else if (strcmp(word, "SELL") == 0) {
        if (sscanf(command, "SELL %i %s %i %i", &order_id, product_name, &quantity, &unit_price) != 4) {
			printf("%s Invalid command format: <%s>\n", LOG_PREFIX , command); 
            SEND_STATUS(t, -1, INVALID);
			return;
        }

        if (order_id != t->next_order_id) {
			printf("%s Order ID %i invalid\n", LOG_PREFIX, order_id);
            SEND_STATUS(t, -1, INVALID);
			return;
        }

        list_node * l = list_find(product_head, product_name);
        if (l == NULL) {
            printf("%s Invalid product name: <%s>\n", LOG_PREFIX ,product_name);
			SEND_STATUS(t, -1, INVALID);
			return;

        }
        product * p = (product *)l->data.product;

        if (quantity < 1 || quantity > 999999) {
			printf("%s Invalid quantity: <%i>\n", LOG_PREFIX, quantity);
            SEND_STATUS(t, -1, INVALID);
			return;
        }

        if (unit_price < 1 || unit_price > 999999) {
			printf("%s Invalid price <$%i>\n", LOG_PREFIX, unit_price);
            SEND_STATUS(t, -1, INVALID);
			return;
        }

        order o = {0};

        o.broker = t;
        o.product = p;
        o.type = SELL;

        o.quantity = quantity;
        o.unit_cost = unit_price;
        o.order_id = order_id;
        o.time = time;

        list_add(&t->orders, &o, ORDER);
        list_add_sorted(&p->sell_orders, &o, ORDER);

        t->next_order_id++;

        //SEND_ACCEPTED
        SEND_STATUS(t, o.order_id, ACCEPTED);

        //SEND_MARKET_UPDATE
        SEND_MARKET_UPDATE(traders, n, o, t);
        //CHECK_MATCH_AND_FILL
    }

    else if (strcmp(word, "AMEND") == 0) {
        if (sscanf(command, "AMEND %i %i %i", &order_id, &quantity, &unit_price) != 3) {
            printf("%s Invalid command format: <%s>\n", LOG_PREFIX , command); 
			SEND_STATUS(t, -1, INVALID);
			return;
        }   

        if (quantity < 1 || quantity > 999999) {
			printf("%s Invalid quantity: <%i>\n", LOG_PREFIX, quantity);
            SEND_STATUS(t, -1, INVALID);
			return;
        }

        if (unit_price < 1 || unit_price > 999999) {
			printf("%s Invalid price <$%i>\n", LOG_PREFIX, unit_price);
            SEND_STATUS(t, -1, INVALID);
			return;
        }

        order * to_amend = find_trader_order(t, order_id);
        if (to_amend == NULL) {
            printf("Failed to find order ID %i\n", order_id);
            exit(2);
        }

        /** Amend order */

        to_amend->time = time;
        to_amend->quantity = quantity;
        to_amend->unit_cost = unit_price;

        //SEND_ACCEPTED
        SEND_STATUS(t, order_id, AMENDED);

        //SEND_MARKET_UPDATE??
        SEND_MARKET_UPDATE(traders, n, *to_amend, t);
        //CHECK_MATCH_AND_FILL
    } 

    else if (strcmp(word, "CANCEL") == 0) {
        if (sscanf(command, "CANCEL %i", &order_id) != 1) {
            printf("%s Invalid command format: <%s>\n", LOG_PREFIX , command); 
			SEND_STATUS(t, -1, INVALID);
			return;
        }

        list_node * to_cancel = find_order_listnode(t, order_id);
        if (to_cancel == NULL) {
			printf("%s Order ID %i invalid\n", LOG_PREFIX, order_id);
            SEND_STATUS(t, -1, INVALID);
			return;
        }

        //update node to cancelled
        order * o = to_cancel->data.order;
        o->type = CANCEL;
        o->quantity = 0;
        o->unit_cost = 0;


        //SEND_CONFIRM
        SEND_STATUS(t, order_id, CANCELLED);
        //MARKET_UPDATE
        SEND_MARKET_UPDATE(traders, n, *o, t);

        // could remove this
        list_delete(&t->orders, to_cancel);

    } else {
        printf("%s Invalid command: <%s>\n", LOG_PREFIX, command);
        SEND_STATUS(t, -1, INVALID);
        return;
    }
}

list_node* init_products(const char * filename) {

    list_node* head = NULL;  
    FILE * myfile;
    if ((myfile = fopen(filename, "r")) == NULL) {
        perror("Error opening product file");
        exit(1);
    }

    int n;
    if (fscanf(myfile, "%d\n", &n) != 1) {
        perror("Failed to read number of products");
        exit(2);
    }

    char buffer[BUFFER_LEN] = {0};

    int i;
    for (i = 0; i < n; i++) {
        fgets(buffer, BUFFER_LEN, myfile);
        buffer[strlen(buffer)-1] = '\0';

        product * p = calloc(1, sizeof(product));
        strcpy(p->name, buffer);
        p->buy_orders = NULL;
        p->sell_orders = NULL;

        list_add(&head, p, PRODUCT);
    }

    printf("%s Trading %d products: ", LOG_PREFIX, i);

    list_node* c = head;
    while(c){
        if (c->next == NULL)
            printf("%s\n", c->data.product->name);
        else   
            printf("%s ", c->data.product->name);
        
        c = c->next;
    }

    fclose(myfile);
    return head;
}

struct trader* get_traders(int argc, char const *argv[], list_node * product_ll) {
    struct trader * traders = malloc((argc - 2) * sizeof(struct trader));

    for (int i = 2; i < argc; i++) {
        traders[i-2].id = i-2;
        strcpy(traders[i-2].path, argv[i]);
        
        traders[i-2].next_order_id = 0;
        traders[i-2].orders = NULL;
		traders[i-2].positions = NULL;

        traders[i-2].online = 0;

        list_node * product_node = product_ll;
        while(product_node != NULL) {
            position * p = malloc(sizeof(position));
            p->broker = &traders[i-2];
            p->item = product_node->data.product;
            p->value = 0;
            p->quantity = 0;

            list_add(&traders[i-2].positions, p, POSITION);
            product_node = product_node->next;
        }
    }

    return traders;
}

void launch(struct trader * t) {
    printf("%s Starting trader %d (%s)\n", LOG_PREFIX, t->id, t->path);

    pid_t pid;

    pid = fork();
    if (pid == -1) {
        perror("Fork() in init_traders failed");
        exit(2);
    }

    if (pid == 0) { // child
        // THIS WILL SHIT ON >9 traders
        char id = t->id + '0'; 

        char * child_args[] = {t->path, &id, NULL};

		// dup2(STDOUT_FILENO, STDOUT_FILENO);
        t->online = 1;
        execv(child_args[0], child_args);

        perror("execv");
        exit(2);

    } else {
        t->pid = pid;
        t->online = 1;
    }

    return;
}

void init_traders(struct trader * traders, int n) {  
    char fifo_path_trader[PATH_LEN] = {0};
    char fifo_path_exchange[PATH_LEN] = {0};
    
    int trader_id;

    for (int i = 0; i < n; i++) {

        /* Create named pipes */

        trader_id = traders[i].id;

        snprintf(fifo_path_exchange, PATH_LEN, FIFO_EXCHANGE, trader_id);
        snprintf(fifo_path_trader, PATH_LEN, FIFO_TRADER, trader_id);

        if (access(fifo_path_exchange, F_OK) == -1) {
            if (mkfifo(fifo_path_exchange, 0666) == -1) { 
                perror("Exchange mkfifo() failed");
                exit(2);
            }
        }

        printf("%s Created FIFO %s\n", LOG_PREFIX, fifo_path_exchange);        

        if (access(fifo_path_trader, F_OK) == -1) {
            if (mkfifo(fifo_path_trader, 0666) == -1) {
                perror("Trader mkfifo() failed");
                exit(2);
            }
        }

        printf("%s Created FIFO %s\n", LOG_PREFIX, fifo_path_trader);

        /* Execute child binary */ 

        launch(&traders[i]);

        /* Connect to pipes */

        int outgoing_fd = open(fifo_path_exchange, O_WRONLY);
        // fcntl(outgoing_fd, F_SETFL, O_NONBLOCK);

        if (outgoing_fd == -1) {
            perror("Open exchange FIFO");
            exit(2);
        }

        printf("%s Connected to %s\n", LOG_PREFIX, fifo_path_exchange);

        int incoming_fd = open(fifo_path_trader, O_RDONLY);
        // fcntl(incoming_fd, F_SETFL, O_NONBLOCK);

        if (incoming_fd == -1) {
            perror("Open trader FIFO");
            exit(2);
        } 

        printf("%s Connected to %s\n", LOG_PREFIX, fifo_path_trader);
        traders[trader_id].incoming_fd = incoming_fd;
        traders[trader_id].outgoing_fd = outgoing_fd;
    }
}

int init_epoll(struct trader * traders, int len) {

    int epoll_instance = epoll_create(len);

    for (int i = 0; i < len; i++) {
        struct epoll_event e = {0};
        e.events = EPOLLIN;

        e.data.fd = traders[i].incoming_fd;

        if (epoll_ctl(epoll_instance, EPOLL_CTL_ADD, traders[i].incoming_fd, &e) == -1) {
            perror("epoll_ctl fail");
            exit(2);
        }
    }

    return epoll_instance;
}

void close_fifos(struct trader * t) {
    close(t->incoming_fd);
    close(t->outgoing_fd);
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

    printf("%s Starting\n", LOG_PREFIX); 
       
   /* Start-up 
    * 1. Read product file 
    * 2. Create named pipes for each trader (>= n)
    * 3. Launch trader as child process, assign trader ID starting from 0
    * 4. After launching each binary, exchange and trader connect to FIFO
    * 5. setup signals 
    * 6. send MARKET_OPEN
    */

    list_node* products_ll = init_products(argv[1]); // get products
    struct trader * traders = get_traders(argc, argv, products_ll); // get list of traders

    /* Signal handling */

    struct sigaction s_sigusr = {0};
    s_sigusr.sa_flags |= SA_SIGINFO;

    s_sigusr.sa_sigaction = signal_handler_read; // could block other signals at this point
    if (sigaction(SIGUSR1, &s_sigusr, NULL) == -1) {
        perror("Failed to bind signal_handler for SIGUSR1");
        exit(2);
    }

    struct sigaction s_sigchld = {0};
    s_sigchld.sa_flags |= SA_SIGINFO;

    s_sigchld.sa_sigaction = signal_handler_disc;
    if (sigaction(SIGCHLD, &s_sigchld, NULL) == -1) {
        perror("Failed to bind signal_handler for SIGCHLD");
        exit(2);
    }

    // if (sigaction(SIGCHLD, &s, NULL) == -1) {
    //     perror("Failed to bind signal_handler for SIGCHLD");
    //     exit(2);
    // }

    /** init traders */
    init_traders(traders, argc - 2); // create trader pipes, fork, fifo connects
   

    SEND_MARKET_OPEN(traders, argc-2);

    /** Main loop
     * 1) Wait for signal 
     * 2) Get trader PID
     * 2) Use epoll to check that trader has actually written to pipe
     * 3) Extract data
     * 4) parse command
     * 5) Update and communicate back to traders
     */

	// sleep(1);

    while(1) {
        if (trader_ready && trader_disconnect == -1)
            pause(); 

        if (trader_ready != -1) {
            trader * sender = find_trader(trader_ready, traders, argc-2);
            if (sender == NULL)
                continue;

            receive_data(sender->incoming_fd, buffer);
            parse_command(sender, buffer, products_ll, traders, argc-2);

            trader_ready = -1;
        }

        if (trader_disconnect != -1) {
            trader * sender = find_trader(trader_disconnect, traders, argc-2);
            if (sender == NULL)
                continue;

            sender->online = 0;

            printf("%s Trader %i disconnected\n", LOG_PREFIX, sender->id);

            trader_disconnect = -1;
        }

        time++;
    }

   /* Endgame
    * - Print [PEX] Trader <Trader ID> disconnected
    * - Reject pending / concurrent orders, maintain existing
    * - [PEX] Trading completed
    * - [PEX] Exchange fees collected: $<total fees>
    */

    /* Close fds */

    for (int i = 0; i < argc-2; i++) {
        struct trader t = traders[i];
        close_fifos(&t);
        list_free(t.orders);
        list_free(t.positions);
    }

    /* Free memory */

    list_free(products_ll); 
    
    /** Free each trader position + order*/

    free(traders);

    return 0;
}