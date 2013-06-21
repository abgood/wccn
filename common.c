#include "check.h"

/* create new socket */
int create_socket(void) {
    int fd = 0;
    int buff = LEN_1024;
    struct protoent *icmp_pro_type = NULL;
    struct timeval time_out = {0};

    /* get ICMP protocol type */
    icmp_pro_type = getprotobyname("icmp");

    /* set socket options */
    fd = socket(PF_INET, SOCK_RAW, icmp_pro_type->p_proto);
    if (fd < 0) {
        out_error("create icmp socket failed!!!\n");
    }

    /* set socket options */
    time_out.tv_sec = ICMP_REQUEST_TIMEOUT;
    setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &buff, sizeof(buff));
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &time_out, sizeof(struct timeval));

    return fd;
}

/* create new address */
struct hostent *get_host_name(char *domain) {
    struct hostent *host_name = NULL;

    /* set socket destination address */
    if ((host_name = gethostbyname(domain)) == NULL) {
        out_error("ping gethostbyname error!!!\n");
    }

    return host_name;
}

/* calc icmp check sum */
unsigned short calc_icmp_cksum(void *packet, int len) {
    unsigned short chksum = 0;
    unsigned short *offset = NULL;
    offset = (unsigned short *)packet;

    while (len > 1) {
        chksum += *offset++;
        len -= sizeof(unsigned short);
    }

    if (len == 1) {
        chksum += *((char *)offset);
    }

    chksum = (chksum >> 16) + (chksum & 0xffff);
    chksum += (chksum >> 16);

    return ~chksum;
}

/* fill in icmp message */
int fill_in_icmp(char *buff, int num, int len) {
    struct icmp *p_icmp = NULL;

    p_icmp = (struct icmp *)buff;
    p_icmp->icmp_type = ICMP_ECHO;
    p_icmp->icmp_code = 0;
    p_icmp->icmp_seq = htons((unsigned short)num);
    p_icmp->icmp_id = htons((unsigned short)getpid());
    p_icmp->icmp_cksum = 0;
    p_icmp->icmp_cksum = calc_icmp_cksum(p_icmp, len + 8);
    return len + 8;
}

/* send icmp message */
int send_icmp(int fd, int len, char *buff, const struct sockaddr_in *dest_addr, icmp_st *i_stat) {
    int ret = 0;

    i_stat->send_num++;
    gettimeofday(&i_stat->send_time, NULL);
    ret = sendto(fd, buff, len, 0, (struct sockaddr *)dest_addr, sizeof(struct sockaddr_in));

    return ret;
}

/* recv icmp message */
int recv_icmp(int fd, char *buff, icmp_st *i_stat, recv_st *r_stat) {
    r_stat->recv_len = 0;
    socklen_t from_len = sizeof(struct sockaddr_in);

    r_stat->recv_len = recvfrom(fd, (void *)buff, LEN_1024, 0, (struct sockaddr *)&r_stat->from_addr, &from_len);
    gettimeofday(&i_stat->recv_time, NULL);

    if (r_stat->recv_len < 0) {
#ifndef WINDOWS
        if (errno == EAGAIN) {
            printf("请求超时.\n");
        } else {
            printf("icmp receive error!!!\n");
        }
#else
        printf("%2d:         *   *    *   *        请求超时.\n",i_stat->send_num);
#endif
        return 1;
    }

    i_stat->recv_num++;
    return 0;
}

/* calc time subversion */
unsigned int time_sub(const struct timeval *recv_time, const struct timeval *send_time) {
    unsigned int sec = 0;
    int u_sec = 0;

    sec = recv_time->tv_sec - send_time->tv_sec;
    u_sec = recv_time->tv_usec - send_time->tv_usec;

    if (u_sec < 0) {
        u_sec += 1000000;
        sec--;
    }

    return sec * 1000 + (unsigned int)(u_sec / 1000);
}

/* set interval time */
void set_time(icmp_st *i_stat) {
    unsigned int interval = 0;

    interval = time_sub(&i_stat->recv_time, &i_stat->send_time);
    i_stat->avg_time = (i_stat->avg_time * (i_stat->send_num - 1) + interval) / i_stat->send_num;

    if (i_stat->min_time > interval) {
        i_stat->min_time = interval;
    }

    if (i_stat->max_time < interval) {
        i_stat->max_time = interval;
    }
}

/* parse ping icmp message */
void parse_ping(char *buff, recv_st *r_stat, icmp_st *i_stat) {
    int ip_head_len = 0;
    int icmp_len = 0;
    struct ip *p_ip = NULL;
    struct icmp *p_icmp = NULL;

    p_ip = (struct ip *)buff;
    ip_head_len = p_ip->ip_hl << 2;
    p_icmp = (struct icmp *)(buff + ip_head_len);
    icmp_len = r_stat->recv_len - ip_head_len;

    if (icmp_len < 8) {
        printf("Bad icmp echo-reply\n");
        return;
    }

    if ((p_icmp->icmp_type != ICMP_ECHOREPLY) || (p_icmp->icmp_id != htons((unsigned short)getpid()))) {
        return;
    }

    printf("%d bytes from %s: icmp_seq=%u ttl=%d time=%d ms\n", icmp_len, inet_ntoa(r_stat->from_addr.sin_addr), ntohs(p_icmp->icmp_seq), p_ip->ip_ttl, time_sub(&i_stat->recv_time, &i_stat->send_time));
}

#ifdef WINDOWS
int gettimeofday(struct timeval *tp, void *tzp) {
    time_t clock;
    struct tm tm;
    SYSTEMTIME wtm;
    GetLocalTime(&wtm);
    tm.tm_year     = wtm.wYear - 1900;
    tm.tm_mon     = wtm.wMonth - 1;
    tm.tm_mday     = wtm.wDay;
    tm.tm_hour     = wtm.wHour;
    tm.tm_min     = wtm.wMinute;
    tm.tm_sec     = wtm.wSecond;
    tm. tm_isdst    = -1;
    clock = mktime(&tm);
    tp->tv_sec = clock;
    tp->tv_usec = wtm.wMilliseconds * 1000;
    return (0);
}
#endif

/* show ping statistics */
void show_ping(icmp_st *i_stat) {
    float send_num, recv_num;
    send_num = i_stat->send_num;
    recv_num = i_stat->recv_num;
    printf("\n--- ping值状态 ---\n");
    printf("%u 个报文被发送, %u 个报文被接收, %0.0f%% 报文丢失\n", (int)send_num, (int)recv_num, (send_num - recv_num) / send_num * 100);
    printf("报文传输: 最长时间 = %.1fms, 最短时间 = %.1fms, 平均时间 = %.2fms\n", i_stat->max_time, i_stat->min_time, i_stat->avg_time);
}

/* parse trace icmp message */
int parse_trace(char *buff, recv_st *r_stat, icmp_st *i_stat) {
    int ip_head_len = 0;
    int icmp_len = 0;
    struct ip *p_ip = NULL;
    struct icmp *p_icmp = NULL;

    p_ip = (struct ip *)buff;
    ip_head_len = p_ip->ip_hl << 2;
    p_icmp = (struct icmp *)(buff + ip_head_len);
    icmp_len = r_stat->recv_len - ip_head_len;

    if (icmp_len < 8) {
        printf("Bad icmp echo-reply\n");
        return 0;
    }

    /* time out icmp message */
    if ((p_icmp->icmp_type == ICMP_TIMEOUT) && (p_icmp->icmp_code == 0)) {
        printf("%2d:        %-15s       %4dms\n", i_stat->send_num, inet_ntoa(r_stat->from_addr.sin_addr), time_sub(&i_stat->recv_time, &i_stat->send_time));
        return 0;
    } else if ((p_icmp->icmp_type == ICMP_ECHOREPLY) || (p_icmp->icmp_id != htons((unsigned short)getpid()))) {
        printf("%2d:        %-15s       %4dms\n", i_stat->send_num, inet_ntoa(r_stat->from_addr.sin_addr), time_sub(&i_stat->recv_time, &i_stat->send_time));
        printf("跟踪完成!\n");
        return 1;
    } else {
        printf("%2d:        目标主机不可达!\n", i_stat->send_num);
        return 1;
    }
    return 0;
}
