#include "check.h"

void out_error(char *fmt, ...) {
    va_list argp;

    va_start(argp, fmt);
    vfprintf(stderr, fmt, argp);
    va_end(argp);

#ifdef WINDOWS
    printf("\n请按任意键退出...");
    getch();
#endif
    exit(1);
}

/* get local domain resolve ip */
char *local_domain_to_ip(char *prefix, char *domain) {
    struct hostent *host_name;
    char *host_ip;

    if ((host_name = gethostbyname(domain)) == NULL) {
        out_error("%s gethostbyname 失败!!!\n", prefix);
    }

    /* player local resolve ip */
    host_ip = inet_ntoa(*((struct in_addr *)host_name->h_addr));

    return host_ip;
}

/* 域名解析检查 */
void chk_resolve(char *domain, char *telecom_ip, char *unicom_ip, char *agent, char *prefix) {
    char *host_ip;

    host_ip = local_domain_to_ip(prefix, domain);

    if (strcmp(agent, "电信") == 0) {
        if (strcmp(host_ip, telecom_ip) == 0) {
            printf("%s 域名解析到 %s ip, 成功!!!\n\n", prefix, agent);
            return;
        }

        if (strcmp(host_ip, unicom_ip) == 0) {
            printf("%s 域名没有解析到 %s ip, 失败!!!\n", prefix, agent);
            goto out_t;
        }

out_t:  out_error("本地正确解析: %s %s\n\n", telecom_ip, domain);
    }

    if (strcmp(agent, "联通") == 0) {
        if (strcmp(host_ip, unicom_ip) == 0) {
            printf("%s 域名解析到 %s ip, 成功!!!\n\n", prefix, agent);
            return;
        }

        if (strcmp(host_ip, telecom_ip) == 0) {
            printf("%s 域名没有解析到 %s ip, 失败!!!\n", prefix, agent);
            goto out_u;
        }

out_u:  out_error("本地正确解析: %s %s\n\n", unicom_ip, domain);
    }
}

/* 检查cdn域名解析ip */
int check_ip(char *host_ip, MYSQL_RES *res, int res_flag) {
    MYSQL_ROW row;
    char cdn_ip[LEN_32] = {0};

    mysql_data_seek(res, 0);

    while ((row = mysql_fetch_row(res))) {
        memset(cdn_ip, '\0', LEN_32);
        strncpy(cdn_ip, row[res_flag], strlen(row[res_flag]));

        if (strcmp(cdn_ip, host_ip) == 0) {
            return 0;
        }
    }
    return 1;
}

/* cdn正确解析ip */
void set_cdn_ip(MYSQL_RES *res, int res_flag) {
    MYSQL_ROW row;

    mysql_data_seek(res, 0);

    printf("本地正确解析(以下任选其一):\n");
    while ((row = mysql_fetch_row(res))) {
        printf("\t\t     %s %s\n", row[res_flag], CDN);
    }
}

/* cdn domain resolve */
void check_cdn(MYSQL_RES *res, char *agent) {
    char *host_ip;

    host_ip = local_domain_to_ip(CDN_PREFIX, CDN);

    if (strcmp(agent, "电信") == 0) {
        if (check_ip(host_ip, res, 1) == 0) {
            printf("%s 域名解析到 %s ip, 成功!!!\n\n", CDN_PREFIX, agent);
            return;
        }

        if (check_ip(host_ip, res, 2) == 0) {
            printf("%s 域名没有解析到 %s ip, 失败!!!\n", CDN_PREFIX, agent);
            goto out_t;
        }

out_t:  set_cdn_ip(res, 1);
        out_error("","");
    }

    if (strcmp(agent, "联通") == 0) {
        if (check_ip(host_ip, res, 2) == 0) {
            printf("%s 域名解析到 %s ip, 成功!!!\n\n", CDN_PREFIX, agent);
            return;
        }

        if (check_ip(host_ip, res, 1) == 0) {
            printf("%s 域名没有解析到 %s ip, 失败!!!\n", CDN_PREFIX, agent);
            goto out_u;
        }

out_u:  set_cdn_ip(res, 2);
        out_error("","");
    }
}

/* domain resolve */
void check_resolve(site_info info, loc_info player, MYSQL_RES *cdn_res) {
    char s_domain[LEN_64] = {0};
    char res_domain[LEN_64] = {0};
    char ass_domain[LEN_64] = {0};

    sprintf(s_domain, "%s%d%s", S_PREFIX, info->site_id, info->domain);
    sprintf(res_domain, "%s%d%s", RES_PREFIX, info->site_id, info->domain);
    sprintf(ass_domain, "%s%d%s", ASS_PREFIX, info->site_id, info->domain);

    /* s域名解析检查 */
    chk_resolve(s_domain, info->telecom_ip, info->unicom_ip, player->agent, S_PREFIX);

    /* res域名解析检查 */
    chk_resolve(res_domain, info->telecom_ip, info->unicom_ip, player->agent, RES_PREFIX);

    /* res域名解析检查 */
    chk_resolve(ass_domain, info->telecom_ip, info->unicom_ip, player->agent, ASS_PREFIX);

    /* cdn域名解析检查 */
    if (info->resource) {
        check_cdn(cdn_res, player->agent);
    }
}
