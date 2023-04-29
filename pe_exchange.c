#include "pe_exchange.h"

node* init_products(const char * filename) {
    node* head = NULL;  

    FILE * myfile = fopen(filename, "r");
    int n;

    if (fscanf(myfile, "%d\n", &n) != 1) {
        perror("Failed to read number of products");
        exit(2);
    }

    char buffer[BUFFER_LEN];

    int i;
    for (i = 0; i < n; i++) {
        fgets(buffer, BUFFER_LEN, myfile);

        buffer[strlen(buffer) - 1] = '\0';
        list_add(&head, buffer);
    }

    printf("%s Trading %d products: ", LOG_PREFIX, i);

    node* c = head;
    while(c){
        printf("%s",c->data);
        c = c->next;
    }

    printf("\n");

    // list_add(&head, "data");

    return head;
}

void setup_pipes(int argc, char const *argv[]) {
    /* 
     * Setup FIFO path for exchange
     * Create fifo pipes for traders
     * Can epoll help here? -> maybe in main
     */
    char fifo_path_trader[PATH_LEN] = {0};
    char fifo_path_exchange[PATH_LEN] = {0};

    int trader_id;

    for (int i = 2; i < argc; i++) { // iterate through trader args
        
        trader_id = i - 2;
        snprintf(fifo_path_exchange, PATH_LEN, FIFO_EXCHANGE, trader_id);
        snprintf(fifo_path_trader, PATH_LEN, FIFO_TRADER, trader_id);

        if (mkfifo(fifo_path_exchange, 0666) == -1) {
            perror("Exchange mkfifo() failed");
            exit(2);
        }

        printf("[SPX] Created FIFO %s\n", fifo_path_exchange);        

        if (mkfifo(fifo_path_trader, 0666) == -1) {
            perror("Trader mkfifo() failed");
            exit(2);
        }

        printf("[SPX] Created FIFO %s\n", fifo_path_trader);
    }
}

int main(int argc, char const *argv[])
{

    if (argc < 2) {
        printf("Invalid launch options\n");
        return 2;
    }

    printf("%s Starting\n", LOG_PREFIX); // custom printf for [SPX] ?

    node* products_ll = init_products(argv[1]);
    
   /* Start-up 
    * 1. Read product file 
    * 2. Create named pipes for each trader (>= n)
    * 3. Launch trader as child process, assign trader ID starting from 0
    * 4. After launching each binary, exchange and trader connect to FIFO
    */

    /* Setup product list */
  

   /* Endgame
    * - Print [SPX] Trader <Trader ID> disconnected
    * - Reject pending / concurrent orders, maintain existing
    * - [SPX] Trading completed
    * - [SPX] Exchange fees collected: $<total fees>
    */

    /* Free memory */

    list_free(products_ll);

    return 0;
}
