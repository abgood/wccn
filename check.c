#include "check.h"

int main (int argc, char **argv) {
    loc_info player_info;
    site_info site_info;
    port_info port_info;
    my_ulonglong lines;

    char site[LEN_16] = {0};
    char site_name[LEN_16] = {0};
    char site_id[LEN_16] = {0};
    char indepe_sql[LEN_256] = {0};
    char common_sql[LEN_256] = {0};
    char cdn_sql[LEN_256] = {0};
    char port_sql[LEN_256] = {0};
    char s_domain[LEN_32] = {0};
    char res_domain[LEN_32] = {0};

    MYSQL_RES *indepe_res;
    MYSQL_RES *common_res;
    MYSQL_RES *cdn_res;
    MYSQL_RES *port_res;

    /* init player_info */
    if (!(player_info = malloc(sizeof(struct local_info)))) {
        out_error("player_info结构动态分配内存失败\n");
    }

    /* init site_info */
    if (!(site_info = malloc(sizeof(struct info_list)))) {
        out_error("site_info结构动态分配内存失败\n");
    }

    /* init port_info */
    if (!(port_info = malloc(sizeof(struct port_list)))) {
        out_error("port_info结构动态分配内存失败\n");
    }

    printf("请输入有问题的游戏区site值: ");
    scanf("%s", site);

    sscanf(site, "%[^_]%*c%s", site_name, site_id);

    /* curl html to get player information */
    curl_data(player_info);
    remove(TMP_FILE);

    /* query indepe table */
    sprintf(indepe_sql, "select * from %s where site like \'%%%s%%\'", INDEPE, site_name);
    indepe_res = quiry(indepe_sql);
    if ((lines = mysql_num_rows(indepe_res)) == (my_ulonglong)0) {
        out_error("无法找到 %s 在 %s 里!!!\n", site, INDEPE);
    }
    /* handle mysql_res */
    handle_indepe(site_info, site_name, atoi(site_id), indepe_res);

    if (strlen(site_info->telecom_ip) < LEN_8) {
        out_error("无法找到 %s 在 %s 里!!!\n", site_id, site_name);
    }

    /* query common table */
    sprintf(common_sql, "select * from %s where site_name like \'%%%s%%\' or site_name like \'%%%s%%\'", COMMON, site_info->site_name, site_name);
    common_res = quiry(common_sql);
    if ((lines = mysql_num_rows(common_res)) == (my_ulonglong)0) {
        out_error("无法找到 %s 在 %s 里!!!\n", site, COMMON);
    }
    /* handle mysql_res */
    handle_common(site_info, site_name, atoi(site_id), common_res);

    /* query port table */
    sprintf(port_sql, "select * from %s", PORTIF);
    port_res = quiry(port_sql);
    if ((lines = mysql_num_rows(port_res)) == (my_ulonglong)0) {
        out_error("无法生成结果在 %s 里!!!\n", PORTIF);
    }
    /* handle mysql_res */
    handle_port(port_info, port_res);

    /* cdn result set */
    sprintf(cdn_sql, "select * from %s", CDNINF);
    cdn_res = quiry(cdn_sql);

    /* domain resolve */
    check_resolve(site_info, player_info, cdn_res);

    /* check ping */
    sprintf(s_domain, "%s%d%s", S_PREFIX, site_info->site_id, site_info->domain);
    check_ping(s_domain, site_info->resource);

    /* check trace */
    check_trace(s_domain, site_info->resource);

    /* check port */
    sprintf(res_domain, "%s%d%s", RES_PREFIX, site_info->site_id, site_info->domain);
    check_port(s_domain, res_domain, site_info->resource, site_info->port, port_info);

    /* win program at the end of the return */
#ifdef WINDOWS
    printf("请按任意键退出...");
    getch();
#endif
    return 0;
}
