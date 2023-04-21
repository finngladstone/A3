#include "pe_trader.h"

int main(int argc, char ** argv) {
    if (argc < 2) {
        printf("Not enough arguments\n");
        return 1;
    }

    int id = atoi(argv[1]);


    // register signal handler

    // connect to named pipes
    

    /* 
    Event loop: 

    wait for exchange update (MARKET message)
    send order
    wait for exchange confirmation (ACCEPTED message)
    
    */

    while(1) {
        pause();
    }
    
}
