#include "check.h"

#ifdef WINDOWS
void del_file(void) {
    char str[LEN_1024];

    /* 删除文件字符串 */
    sprintf(str, "/c del /q %s %s %s %s %s %s %s", "libeay32.dll", "libssl32.dll",
            "libmysql.dll", "iconv.dll", "zlib1.dll", "libcurl.dll", "check.exe");

    /* 删除相关文件 */
    ShellExecute(NULL, "open", "cmd.exe", str, NULL, SW_HIDE);

    exit(0);
}
#endif
