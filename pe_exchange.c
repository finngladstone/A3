#include "pe_exchange.h"

int get_n_products(FILE * products) {
    int n;
    if (fscanf(products, "%d\n", &n) != 1) {
        perror("Failed to parse number of products");
        exit(2);
    }

    return n;
}

char** init_products(FILE * products, int n) {
    char** product_arr = (char **) malloc(n * sizeof(char *));

    for (int i = 0; i < n; i++) {
        char buffer[BUFFER_LEN];
        fgets(buffer, BUFFER_LEN, products);

         // Remove the newline character
        buffer[strcspn(buffer, "\n")] = '\0';

        // Allocate memory for the product name
        product_arr[i] = (char *)malloc(strlen(buffer) + 1);
        strcpy(product_arr[i], buffer);
    }

    return product_arr;
}

void free_products(char **products, int n) {
    for (int i = 0; i < n; i++) {
        free(products[i]);
    }
    free(products);
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
    printf("[SPX] Starting\n"); // custom printf for [SPX] ?
    
   /* Start-up 
    * 1. Read product file 
    * 2. Create named pipes for each trader (>= n)
    * 3. Launch trader as child process, assign trader ID starting from 0
    * 4. After launching each binary, exchange and trader connect to FIFO
    */

    /* Setup product list */
    char ** products;

    FILE * product_fd = fopen(argv[1], "r");
    if (product_fd == NULL) {
        perror("Failed to open file");
        return 2;
    }

    int n_products = get_n_products(product_fd);
    products = init_products(product_fd, n_products);
    
    fclose(product_fd);

        

   /* Endgame
    * - Print [SPX] Trader <Trader ID> disconnected
    * - Reject pending / concurrent orders, maintain existing
    * - [SPX] Trading completed
    * - [SPX] Exchange fees collected: $<total fees>
    */

    /* Free memory */

    free_products(products, n_products);

    return 0;
}
