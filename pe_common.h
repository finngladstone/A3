#ifndef PE_COMMON_H
#define PE_COMMON_H

#define _POSIX_SOURCE
#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>
#include <sys/wait.h>

#define FIFO_EXCHANGE "/tmp/pe_exchange_%d"
#define FIFO_TRADER "/tmp/pe_trader_%d"
#define FEE_PERCENTAGE 1

#endif
