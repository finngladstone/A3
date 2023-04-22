#include "pe_trader.h"

void signal_handler(int s, siginfo_t* sinfo, void * context) {
    
}

// int place_order() {}

int main(int argc, char ** argv) {
    if (argc < 2) {
        printf("Not enough arguments\n");
        return 1;
    }

    pid_t exchange_pid = getppid();

    struct sigaction s = {0};
    s.sa_flags |= SA_SIGINFO;


    // connect to named pipes

    int fd_write = open(FIFO_TRADER, O_WRONLY);
    int fd_read = open(FIFO_EXCHANGE, O_RDONLY);
    

    /* Collect MARKET OPEN */

    pause();

    /* 
    Event loop: 

    wait for exchange update (MARKET message)
    send order
    wait for exchange confirmation (ACCEPTED message)
    
    */

    while(1) {
    
    }
    
}
