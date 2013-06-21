#ifndef CHECK_COMMON_H
#define CHECK_COMMON_H

#ifdef WINDOWS

#define ICMP_ECHO 8
#define ICMP_ECHOREPLY 0

typedef struct icmp 
{
    BYTE   icmp_type;
    BYTE   icmp_code;                 // Type sub code
    USHORT icmp_cksum;
    USHORT icmp_id;
    USHORT icmp_seq;
    ULONG  timestamp;
} icmp;

// IP header structure
typedef struct ip
{
    //Suppose the BYTE_ORDER is LITTLE_ENDIAN
    unsigned int   ip_hl:4;        // Length of the header
    unsigned int   version:4;      // Version of IP
    unsigned char  tos;            // Type of service
    unsigned short total_len;      // Total length of the packet
    unsigned short id;             // Unique identification
    unsigned short frag_offset;    // Fragment offset
    unsigned char  ip_ttl;            // Time to live
    unsigned char  protocol;       // Protocol (TCP, UDP etc)
    unsigned short checksum;       // IP checksum
    unsigned int   sourceIP;       // Source IP
    unsigned int   destIP;         // Destination IP
} ip;

int gettimeofday(struct timeval *, void *);
#endif

typedef struct icmp_stat {
    unsigned int send_num;
    unsigned int recv_num;
    struct timeval send_time;
    struct timeval recv_time;
    float max_time;
    float min_time;
    float avg_time;
} icmp_st;

/* create socket */
int create_socket(void);

/* get host name */
struct hostent *get_host_name(char *);

/* file in icmp message */
int fill_in_icmp(char *, int, int);

/* calc icmp check sum */
unsigned short calc_icmp_cksum(void *, int);

/* set interval time */
void set_time(icmp_st *);

/* show ping statistics */
void show_ping(icmp_st *);

#endif
