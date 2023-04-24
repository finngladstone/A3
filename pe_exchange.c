#include "pe_exchange.h"

void setup_pipes(int argc, char const *argv[]) {
    /* 
     * Connect each trader to a named pipe (FIFO)
     * Can epoll help here?
     */
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
