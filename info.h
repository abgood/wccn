#ifndef CHECK_INFO_H
#define CHECK_INFO_H

/* site info */
typedef struct info_list {   
    char *site_name;     /* 游戏site_name */    
    char *telecom_ip;    /* 电信ip */
    char *unicom_ip;     /* 联通ip */    
    char *domain;        /* 域名后半段 */    
    int site_id;                /* 游戏site_id */
    int port;                   /* 游戏端口 */    
    int resource;               /* res本地资源域名, cdn域名 */    
} *site_info;

#endif
