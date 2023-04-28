#include "pe_exchange.h"

void setup_pipes(int argc, char const *argv[]) {
    /* 
     * Setup FIFO path for exchange
     * Create fifo pipes for traders
     * Can epoll help here? -> maybe in main
     */
    char fifo_path[PATH_LEN] = {0};
    int trader_id;

    for (int i = 2; i < argc; i++) { // iterate through trader args
        
        trader_id = i - 2;
        snprintf(fifo_path, PATH_LEN, FIFO_TRADER, trader_id);

        if (mkfifo(fifo_path, 0666) == -1) {
            perror("mkfifo failed");
            exit(2);
        }
    }

    // setup exchange fifo

    


}

int main(int argc, char const *argv[])
{
    
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
