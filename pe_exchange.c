#include "pe_exchange.h"

/** 
 * Open fd for products list 
 * Initialise LL for products using Richard's template
 * Output status message
 * Need to call list_free() upon endgame!
 */

void signal_handler(int s, siginfo_t* sinfo, void * context) {
    // int who = sinfo->si_pid;
    // [SPX] [T0] Parsing command: <BUY 0 GPU 30 500>
}

product_node* init_products(const char * filename) {

    product_node* head = NULL;  
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
        buffer[strlen(buffer)-2] = '\0';

        product_list_add(&head, buffer);
    }

    printf("%s Trading %d products: ", LOG_PREFIX, i);

    product_node* c = head;
    while(c){
        if (c->next == NULL)
            printf("%s\n", c->data);
        else   
            printf("%s ", c->data);
        
        c = c->next;
    }

    fclose(myfile);
    return head;
}

trader* get_traders(int argc, char const *argv[]) {
    trader * traders = malloc((argc - 2) * sizeof(trader));

    for (int i = 2; i < argc; i++) {
        traders[i-2].id = i-2;
        strcpy(traders[i-2].path, argv[i]);
    }

    return traders;
}

void launch(trader * t) {
    printf("%s Starting trader %d (%s)\n", LOG_PREFIX, t->id, t->path);

    pid_t pid;

    pid = fork();
    if (pid == -1) {
        perror("Fork() in init_traders failed");
        exit(2);
    }

    if (pid == 0) { // child
        
        char id_string[3] = {0};
        snprintf(id_string, 2, "%d", t->id);

        if (execl(t->path, id_string) == -1) {
            perror("execl failed");
            exit(2);
        }
    } else {
        t->pid = pid;
    }

    return;
}

void init_traders(trader * traders, int n) {  
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

        int incoming_fd = open(fifo_path_exchange, O_RDONLY);
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

int init_epoll(trader * traders, int len) {

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

void close_fifos(trader * t) {
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

    printf("%s Starting\n", LOG_PREFIX); 
       
   /* Start-up 
    * 1. Read product file 
    * 2. Create named pipes for each trader (>= n)
    * 3. Launch trader as child process, assign trader ID starting from 0
    * 4. After launching each binary, exchange and trader connect to FIFO
    */

    product_node* products_ll = init_products(argv[1]); // get products
    trader * traders = get_traders(argc, argv); // get list of traders

    init_traders(traders, argc - 2); // create trader pipes, fork, fifo connects

    /* Epoll setup */ 

    // int epoll_fd;
    // epoll_fd = init_epoll(traders, argc - 2);

    // struct epoll_event * events = malloc(argc - 2 * sizeof(struct epoll_event));
    
    // while(1) {
    //     int num_events = epoll_wait(epoll_fd, events, argc-2, -1);

    //     for (int i = 0; i < num_events; i++) {

    //     }
    // }

    /* Signal handling */

    struct sigaction s = {0};
    s.sa_flags |= SA_SIGINFO;

    s.sa_sigaction = signal_handler; // could block other signals at this point

    // MARKET_OPEN(traders, argc-2);

    /** Main loop
     * 1) Wait for signal 
     * 2) Get trader PID
     * 2) Use epoll to check that trader has actually written to pipe
     * 3) Extract data
     * 4) parse command
     * 5) Update and communicate back to traders
     */

    // while(1) {
    //     pause(); 
    // }


    

    

   /* Endgame
    * - Print [PEX] Trader <Trader ID> disconnected
    * - Reject pending / concurrent orders, maintain existing
    * - [PEX] Trading completed
    * - [PEX] Exchange fees collected: $<total fees>
    */

    /* Close fds */

    for (int i = 0; i < argc-2; i++) {
        trader t = traders[i];
        close_fifos(&t);
    }

    /* Free memory */

    product_list_free(products_ll); 
    free(traders);
    // free(events);

    return 0;
}
