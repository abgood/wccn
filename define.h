#ifndef CHECK_DEFINE_H
#define CHECK_DEFINE_H

#include <mysql/mysql.h>

/* set the platform
 * set is win
 * no set is linux
 */
// #define WINDOWS

/* Length */
#define LEN_8 8
#define LEN_16 16
#define LEN_32 32
#define LEN_64 64
#define LEN_128 128
#define LEN_256 256
#define LEN_512 512
#define LEN_1024 1024
#define LEN_4096 4096
#define LEN_10240 10240

/* table */
#define INDEPE "indepe_info"
#define COMMON "common_info"
#define CDNINF "cdn_info"
#define PORTIF "port_info"

/* some file */
#define TMP_FILE "tmp"

/* 分割符 */
#define D_SITE_NAME "="
#define D_SITE_ID "-"
#define D_ID_FIELD ","
#define D_ID_RANGE "~"

/* 前缀 */
#define S_PREFIX "s"
#define RES_PREFIX "res"
#define ASS_PREFIX "assist"
#define CDN_PREFIX "cdn"
#define WEBPORT 80
#define FLAPORT 843
#define CDNPORT 8080

/* cdn domain */
#define CDN "cdn.shenqu.7road.net"

/* ICMP protocol set */
#define ICMP_REQUEST_TIMEOUT 2
#define ICMP_DATA_LEN 20
#define ICMP_ECHO_MAX 10
#define ICMP_HOP_MAX 30
#define ICMP_TIMEOUT 11

/* local struct */
typedef struct local_info {
    char *ip;
    char *address;
    char *agent;
} *loc_info;

/* port info */
typedef struct port_list {
    int base_port;
    int time_port;
} *port_info;

#endif
