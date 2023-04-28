#include "pe_exchange.h"

// char** get_products(char * filename) {
//     char** products;
//     int n_products;

//     FILE * file = fopen(filename, "r");
//     if (file == NULL) {
//         perror("Failed to read products list");
//         exit(2);
//     }

//     fscanf(file, "%d\n", &n_products); // read product len
//     products = (char **) malloc(n_products * sizeof(char *));

//     for (int i = 0; i < n_products; i++) {
//         char buffer[BUFFER_LEN];
//         fgets(buffer, BUFFER_LEN, file);

//         buffer[strcspn(buffer, "\n")] = '\0';

//         products[i] = (char *) malloc(strlen(buffer) + 1);
//         strcpy(products[i], buffer);
//     }

//     /* Verbose product reading */
//     printf("[SPX] Trading %d products: ", n_products);
//     for (int i = 0; i < n_products; i++) {
//         printf("%s", products[i]);
        
//         if (i < n_products - 1) {
//             printf(" ");
//         }
//     }

//     printf("\n");

//     return products;
// }

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

   /* Endgame
    * - Print [SPX] Trader <Trader ID> disconnected
    * - Reject pending / concurrent orders, maintain existing
    * - [SPX] Trading completed
    * - [SPX] Exchange fees collected: $<total fees>
    */

    return 0;
}
