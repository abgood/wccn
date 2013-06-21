#ifndef CHECK_HANDLE_H
#define CHECK_HANDLE_H

#include "info.h"
#include "define.h"

/* handle indepe_info */
void handle_indepe(site_info, char *, int, MYSQL_RES *);

/* handle common_info */
void handle_common(site_info, char *, int, MYSQL_RES *);

/* handle port_info */
void handle_port(port_info, MYSQL_RES *);

#endif
