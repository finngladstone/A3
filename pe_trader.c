#include "pe_trader.h"

void signal_handler(int s, siginfo_t* sinfo, void * context) {

}

void write_data(int fd, char * message) {}

void read_data(int fd, char * buffer) {}

// int place_order() {}

int main(int argc, char ** argv) {
    if (argc < 2) {
        printf("Not enough arguments\n");
        return 1;
    }

    char buffer[BUFFER_SIZE];

    int fd_write;
    int fd_read;

    // connect to named pipes
    if ((fd_write = open(FIFO_TRADER, O_WRONLY)) == -1) 
        perror("Failed to open FIFO_TRADER");
    
    if ((fd_read = open(FIFO_EXCHANGE, O_RDONLY)) == -1) 
        perror("Failed to open EXCHANGE_FIFO");
    
    /* Collect MARKET OPEN 
    - read and check message
    - if valid, proceed to main loop
    */

    pause();

    /* 
    Event loop: 

    wait for exchange update (MARKET message)
    send order
    wait for exchange confirmation (ACCEPTED message)
    
    */

    while(1) {
    
    }

    close(fd_read);
    close(fd_write);
    
}
