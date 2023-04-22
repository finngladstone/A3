#include "pe_trader.h"

void signal_handler(int s, siginfo_t* sinfo, void * context) {
    if (s == SIGUSR1)
        printf("Trader received SIGUSR1 signal\n");
}

void signal_h(int s) {}

void write_data(int fd, char * message) {}

void read_data(int fd, char * buffer) {
    ssize_t n_read;
    if ((n_read = read(fd, &buffer, BUFFER_SIZE-1)) == 1) {
        perror("read_data fail");
        exit(2);
    }

    buffer[n_read] = '\0';
}

// int place_order() {}

int main(int argc, char ** argv) {
    if (argc < 2) {
        printf("Not enough arguments\n");
        return 1;
    }

    int self_id = atoi(argv[1]);

    /* FIFO filename setup */

    char read_path[PATH_LEN];
    char write_path[PATH_LEN];

    snprintf(read_path, PATH_LEN, FIFO_EXCHANGE, self_id);
    snprintf(write_path, PATH_LEN, FIFO_TRADER, self_id);

    char buffer[BUFFER_SIZE];

    struct sigaction s = { 0 };
    s.sa_flags |= SA_SIGINFO; // what even the fuck does this do

    int fd_write;
    int fd_read;

    /* Setup and handle potential read errors for FIFO pipes*/

    if ((fd_read = open(read_path, O_RDONLY)) == -1) 
        perror("Failed to open EXCHANGE_FIFO");

    if ((fd_write = open(write_path, O_WRONLY)) == -1) 
        perror("Failed to open FIFO_TRADER");
    
    /* Setup signal handling */

    s.sa_sigaction = signal_handler;
    signal(SIGUSR1, signal_h);
    
    /* Collect MARKET OPEN 
    - read and check message
    - if valid, proceed to main loop
    */

    pause();
    read_data(fd_read, buffer);

    if (strcmp(buffer, "MARKET OPEN") == 0) {
        printf("T[] Received MARKET OPENED.. proceeding\n");
    } else {
        perror("Unexpected token");
    }

    /* 
    Event loop: 

    wait for exchange update (MARKET message)
    send order
    wait for exchange confirmation (ACCEPTED message)
    
    */

    // while(1) {
    
    // }

    
    /* End of program cycle */

    close(fd_read);
    close(fd_write);

    return 0;
    
}
