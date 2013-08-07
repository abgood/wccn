#include "check.h"

/* handle common_info */
void handle_common(site_info output, char *input_name, int input_id, MYSQL_RES *res) {
    MYSQL_ROW row;
    int lines = 0;

    /* site_info 动态分配domain内存 */
    output->domain = (char *)malloc(LEN_64 * sizeof(char));
    /* 取得结果集行数 */
    lines = (int)mysql_num_rows(res);

    /* 一行结果集表明从区不需要解析,用的是主区的域名解析 */
    if (lines == 1) {
        while ((row = mysql_fetch_row(res))) {
            memset(output->domain, '\0', LEN_64);
            if (!strcmp(output->site_name, row[1])) {
                strncpy(output->domain, row[2], strlen(row[2]));
            }
        }
    } else if (lines == 2) {    /* 两行结果集表明代理各自解析 */
        while ((row = mysql_fetch_row(res))) {
            memset(output->domain, '\0', LEN_64);
            if (!strcmp(input_name, row[1])) {
                strncpy(output->domain, row[2], strlen(row[2]));
                output->site_id = input_id;
                return;
            }
        }
    } else {
        printf("handle_common no results\n");
    }
}

/* handle indepe_info */
void handle_indepe(site_info output, char *input_name, int input_id, MYSQL_RES *res) {
    MYSQL_ROW row;
    char agent[LEN_1024] = {0};
    char m_name[LEN_64] = {0};
    char m_id[LEN_32] = {0};
    char id_token_start[LEN_512] = {0};
    char id_token_end[LEN_512] = {0};
    char id_start_tmp[LEN_512] = {0};
    char *s_n_end;
    char *s_i_end;
    char *i_n_start;
    char *m_i_start;
    char *id_start;
    char *token;
    int int_id_start;
    int int_id_end;

    /* site_info 动态分配site_name内存 */
    output->site_name = (char *)malloc(LEN_16 * sizeof(char));

    while ((row = mysql_fetch_row(res))) {
        memset(output->site_name, '\0', LEN_16);
        memset(agent, '\0', LEN_1024);
        memset(m_id, '\0', LEN_32);
        memset(m_name, '\0', LEN_64);
        strncpy(agent, row[1], strlen(row[1]));
        
        /* get master site_name */
        if ((s_n_end = strstr(agent, D_SITE_NAME))) {
            strncpy(m_name, agent, s_n_end - agent);
        }
        strncpy(output->site_name, m_name, strlen(m_name));

        /* get master site_id */
        if ((s_i_end = strstr(agent, D_SITE_ID))) {
            strncpy(m_id, s_n_end + 1, s_i_end - s_n_end - 1);
        } else {
            strcpy(m_id, s_n_end + 1);
        }

        /* find input_id in record */
        i_n_start = strstr(agent, input_name);
        m_i_start = strstr(i_n_start, D_SITE_NAME);
        /* find each site_name from id to start */
        if ((id_start = strstr(m_i_start, D_SITE_ID))) {
            id_start += 1;
        } else {
            id_start = m_i_start + 1;
        }

        /* if have not "~", compare id fields */
        if (!strstr(id_start, D_ID_RANGE)) {
            if (input_id == atoi(id_start)) {
                output->telecom_ip = row[2];
                output->unicom_ip = row[3];
                output->port = atoi(row[4]);
                output->resource = atoi(row[5]);
                output->site_id = atoi(m_id);
                return;
            }
        } else {
            memset(id_start_tmp, '\0', LEN_512);
            strncpy(id_start_tmp, id_start, strlen(id_start));
            token = strtok(id_start_tmp, D_ID_FIELD);
            while (token) {
                memset(id_token_start, '\0', LEN_512);
                memset(id_token_end, '\0', LEN_512);
                sscanf(token, "%[^~]~%[^;];", id_token_start, id_token_end);

                int_id_start = atoi(id_token_start);
                int_id_end = atoi(id_token_end);

                /* get ip */
                if ((input_id >= int_id_start && input_id <= int_id_end) || (input_id == int_id_start)) {
                    output->telecom_ip = row[2];
                    output->unicom_ip = row[3];
                    output->port = atoi(row[4]);
                    output->resource = atoi(row[5]);
                    output->site_id = atoi(m_id);
                    return;
                }

                token = strtok(NULL, D_ID_FIELD);
            }
        }
    }
}

/* handle port_info */
void handle_port(port_info port, MYSQL_RES *res) {
    MYSQL_ROW row;

    while ((row = mysql_fetch_row(res))) {
        port->base_port = atoi(row[1]);
        port->time_port = atoi(row[2]);
    }
}
