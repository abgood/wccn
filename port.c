#include "check.h"

void conn_port(char *domain, int port) {
    int fd;
    struct sockaddr_in addr;
    struct hostent *host_name = NULL;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        out_error("create check port %d socket error!!!\n", port);
    }
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    host_name = get_host_name(domain);
    memcpy((char *)&addr.sin_addr, (char *)(host_name->h_addr), host_name->h_length);
    addr.sin_family = host_name->h_addrtype;
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        printf("连接 %s 的 %d 端口失败!!!\n", domain, port);
    } else {
        printf("连接 %s 的 %d 端口成功!!!\n", domain, port);
    }

    close(fd);
}

void chk_port(char *domain, char *cdn, port_info port_info, int site_port) {
    int i;
    int from_port;
    int to_port;

    from_port = port_info->base_port + (port_info->time_port * site_port);
    to_port = port_info->base_port + (port_info->time_port * site_port) + (port_info->time_port - 1);

    printf("\n");

    /* game port check*/
    for (i = from_port; i <= to_port; i++) {
        conn_port(domain, i);
    }

    printf("\n");

    /* nginx port check */
    conn_port(domain, WEBPORT);

    printf("\n");

    /* flash port check */
    conn_port(domain, FLAPORT);

    printf("\n");

    /* resource port check */
    conn_port(cdn, CDNPORT);

    printf("\n");
}

void check_port(char *s_domain, char *res_domain, int res_flag, int site_port, port_info port_info) {
    /* res domain port */
    if (res_flag) {
        chk_port(s_domain, CDN, port_info, site_port);
    } else {
        /* s domain port */
        chk_port(s_domain, res_domain, port_info, site_port);
    }
}
