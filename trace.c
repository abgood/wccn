#include "check.h"

void chk_trace(char *domain) {
    int fd = 0;
    int echo_num = 1;
    int send_len = 0;

    icmp_st i_stat;
    recv_st r_stat;

    struct sockaddr_in addr;
    struct hostent *host_name = NULL;

    char send_buff[LEN_1024];
    char recv_buff[LEN_1024];

    i_stat.send_num = 0;
    i_stat.recv_num = 0;
    i_stat.max_time = -1.0;
    i_stat.min_time = 1000000.0;
    i_stat.avg_time = 0.0;

    /* create socket */
    fd = create_socket();

    /* get host name */
    host_name = get_host_name(domain);

    memcpy((char *)&addr.sin_addr, (char *)(host_name->h_addr), host_name->h_length);
    addr.sin_family = host_name->h_addrtype;

    printf("\ntraceroute to %s (%s), %d hops max, %d byte packets\n", domain, inet_ntoa(addr.sin_addr), ICMP_HOP_MAX, ICMP_DATA_LEN);

    while (echo_num <= ICMP_HOP_MAX) {
        memset(send_buff, 0, sizeof(send_buff));
        memset(recv_buff, 0, sizeof(recv_buff));
        memset(&i_stat.send_time, 0, sizeof(i_stat.send_time));
        memset(&i_stat.recv_time, 0, sizeof(i_stat.recv_time));
        memset(&r_stat.from_addr, 0, sizeof(r_stat.from_addr));

        /* set icmp ttl */
        setsockopt(fd, IPPROTO_IP, IP_TTL, (char *)&echo_num, sizeof(unsigned int));

        /* fill in icmp message */
        send_len = fill_in_icmp(send_buff, echo_num, ICMP_DATA_LEN);

        echo_num++;

        /* send icmp message */
        if (send_icmp(fd, send_len, send_buff, &addr, &i_stat) < 0) {
            continue;
        }

        /* recv icmp message */
        if (recv_icmp(fd, recv_buff, &i_stat, &r_stat)) {
            continue;
        }

        /* set time */
        set_time(&i_stat);

        /* parse trace icmp message */
        if (parse_trace(recv_buff, &r_stat, &i_stat)) {
            break;
        }

        sleep(1);
    }
}

void check_trace(char *domain, int res_flag) {
    /* res domain trace */
    if (res_flag) {
        chk_trace(CDN);
    }
    /* s domain trace */
    chk_trace(domain);
}
