#ifndef MONGOOSE_CUSTOM_H
#define MONGOOSE_CUSTOM_H

#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Ethernet/wizchip_conf.h"
#include "Ethernet/socket.h"

#define MG_ARCH MG_ARCH_CUSTOM
#define MG_ENABLE_SOCKET 1
#define MG_ENABLE_CUSTOM_MILLIS 1
#define MG_CUSTOM_NONBLOCK(fd)
#define mkdir(a, b) (-1)
//doesn't matter just allow compilation
#define TCP_NODELAY 0
#define SOL_SOCKET 0
#define SO_KEEPALIVE 0
#define getpeername(a, b, c) (0)

#endif