#ifndef CHECK_RESOLVE_H
#define CHECK_RESOLVE_H

#include "info.h"
#include "define.h"

/* domain resolve */
void check_resolve(site_info, loc_info, MYSQL_RES *);

/* windows error exit */
void out_error(char *, ...);

#endif
