#include "check.h"

void out_error(char *fmt, ...) {
    va_list argp;
    FILE *fp = fopen(OUT_FILE, "a");

    va_start(argp, fmt);
    vfprintf(stderr, fmt, argp);
    vfprintf(fp, fmt, argp);

    fclose(fp);
    va_end(argp);

#ifdef WINDOWS
    if (show == 1) {
        printf("请拷贝上面的正确内容保存并按任意键继续 ...");
        getch();
    }
#endif

    /* 跳转到跳转点处 */
    printf("\n\n");
    longjmp(jmpbuffer, 1);
}

void out_write(char *fmt, ...) {
    va_list argp;
    FILE *fp = fopen(OUT_FILE, "a");

    va_start(argp, fmt);
    vfprintf(stdout, fmt, argp);
    vfprintf(fp, fmt, argp);

    fclose(fp);
    va_end(argp);
}

/* get local domain resolve ip */
char *local_domain_to_ip(char *prefix, char *domain, char *agent, char *telecom_ip, char *unicom_ip) {
    struct hostent *host_name;
    char *host_ip;

    if ((host_name = gethostbyname(domain)) == NULL) {
        out_write("%s gethostbyname失败,本地DNS失败!!!\n", prefix);
        if (strstr(agent, "电信")) {
            show = 1;
            out_error("本地正确解析: %s %s\n\n", telecom_ip, domain);
        } else if (strstr(agent, "联通")) {
            show = 1;
            out_error("本地正确解析: %s %s\n\n", unicom_ip, domain);
        } else {
            show = 1;
            out_error("本地正确解析: %s %s\n\n", unicom_ip, domain);
        }
    }

    /* player local resolve ip */
    host_ip = inet_ntoa(*((struct in_addr *)host_name->h_addr));

    return host_ip;
}

/* 域名解析检查 */
void chk_resolve(char *domain, char *telecom_ip, char *unicom_ip, char *agent, char *prefix) {
    char *host_ip;

    host_ip = local_domain_to_ip(prefix, domain, agent, telecom_ip, unicom_ip);

    if (strstr(agent, "电信")) {
        if (strcmp(host_ip, telecom_ip) == 0) {
            out_write("%s 域名解析到 %s ip, 成功!!!\n\n", prefix, agent);
            return;
        }

        if (strcmp(host_ip, unicom_ip) == 0) {
            out_write("%s 域名没有解析到 %s ip, 失败!!!\n", prefix, agent);
            goto out_t;
        }

out_t:  show = 1;
        out_error("本地正确解析: %s %s\n\n", telecom_ip, domain);
    }

    if (strstr(agent, "联通")) {
        if (strcmp(host_ip, unicom_ip) == 0) {
            out_write("%s 域名解析到 %s ip, 成功!!!\n\n", prefix, agent);
            return;
        }

        if (strcmp(host_ip, telecom_ip) == 0) {
            out_write("%s 域名没有解析到 %s ip, 失败!!!\n", prefix, agent);
            goto out_u;
        }

out_u:  show = 1;
        out_error("本地正确解析: %s %s\n\n", unicom_ip, domain);
    }

    if (!strstr(agent, "电信") && !strstr(agent, "联通")) {
        out_write("您本地的网络既不是电信也不是联通,而是 %s \n", agent);
        return;
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

    out_write("本地正确解析(以下任选其一):\n");
    while ((row = mysql_fetch_row(res))) {
        out_write("\t\t     %s %s\n", row[res_flag], CDN);
    }

    show = 1;

    out_error("\n");
}

/* cdn domain resolve */
void check_cdn(MYSQL_RES *res, char *agent) {
    char *host_ip;

    host_ip = local_domain_to_ip(CDN_PREFIX, CDN, agent, CDN_TC_IP, CDN_UN_IP);

    if (strcmp(agent, "电信") == 0) {
        if (check_ip(host_ip, res, 1) == 0) {
            out_write("%s 域名解析到 %s ip, 成功!!!\n\n", CDN_PREFIX, agent);
            return;
        }

        if (check_ip(host_ip, res, 2) == 0) {
            out_write("%s 域名没有解析到 %s ip, 失败!!!\n", CDN_PREFIX, agent);
            goto out_t;
        }

out_t:  set_cdn_ip(res, 1);
    }

    if (strcmp(agent, "联通") == 0) {
        if (check_ip(host_ip, res, 2) == 0) {
            out_write("%s 域名解析到 %s ip, 成功!!!\n\n", CDN_PREFIX, agent);
            return;
        }

        if (check_ip(host_ip, res, 1) == 0) {
            out_write("%s 域名没有解析到 %s ip, 失败!!!\n", CDN_PREFIX, agent);
            goto out_u;
        }

out_u:  set_cdn_ip(res, 2);
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

    /* ass域名解析检查 */
    chk_resolve(ass_domain, info->telecom_ip, info->unicom_ip, player->agent, ASS_PREFIX);

    /* cdn域名解析检查 */
    if (info->resource) {
        check_cdn(cdn_res, player->agent);
    } else {
        /* res域名解析检查 */
        chk_resolve(res_domain, info->telecom_ip, info->unicom_ip, player->agent, RES_PREFIX);
    }

    show = 0;
}
