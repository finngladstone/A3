### Describe how your exchange works.

The exchange makes heavy use of linked lists (dynamically allocated) to store data for Products, Orders and (trader) Positions. 

I use a queue to receive trader signals. The queue is updated by the methods `signal_handler_read` and `signal_handler_disc` and the global variable `queue_size` .

To wait for signals to arrive, I check that the queue is empty and that we still have live traders in the system.  I then `pause()` to avoid heavy CPU usage and lift my signal masks until a signal arrives. 

The signal masks prevent any incoming signals from interrupting execution until we are ready to handle them. 



### Describe your design decisions for the trader and how it's fault-tolerant.

I ensured that any command that the trader received from the exchange that did not meet the form `MARKET SELL [name] [quantity] [price];` was flagged as invalid by using the `%n` flag in `sscanf` to check for string termination. 

Regarding signalling, I used `sigprocmask` and `sigsuspend` to wait for SIGUSR1 signals until they arrived. This would prevent excessive CPU usage (as it is passive polling). It would also eliminate the chance of a race condition occurring between checking for an event and then pausing the process. 



### Describe your tests and how to run them.

No attempt
