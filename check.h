#ifndef CHECK_H
#define CHECK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <iconv.h>
#include <stdarg.h>
#include <ctype.h>

#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>
#include <openssl/aes.h>

#include "define.h"
#include "curldata.h"
#include "conver.h"
#include "quiry.h"
#include "handle.h"
#include "resolve.h"
#include "aes_code.h"
#include "ping.h"
#include "trace.h"
#include "common.h"
#include "port.h"

#ifndef WINDOWS
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

typedef struct recv_stat {
    int recv_len;
    struct sockaddr_in from_addr;
} recv_st;

/* send icmp message */
int send_icmp(int, int, char *, const struct sockaddr_in *, icmp_st *);

/* recv icmp message */
int recv_icmp(int, char *, icmp_st *, recv_st *);

/* parse ping icmp message */
void parse_ping(char *, recv_st *, icmp_st *);

/* parse trace icmp message */
int parse_trace(char *, recv_st *, icmp_st *);

#endif
