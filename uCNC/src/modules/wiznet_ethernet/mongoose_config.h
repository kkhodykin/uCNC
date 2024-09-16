#ifndef MONGOOSE_CUSTOM_H
#define MONGOOSE_CUSTOM_H

#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "Ethernet/wizchip_conf.h"
#include "Ethernet/socket.h"

#define MG_ARCH MG_ARCH_CUSTOM
#define MG_TLS MG_TLS_BUILTIN
#define MG_ENABLE_TCPIP 1
#define MG_ENABLE_CUSTOM_MILLIS 1
#define MG_ENABLE_PACKED_FS 1
#define MG_ENABLE_POSIX_FS 0
#define FD_SETSIZE 128

#endif