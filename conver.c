#include "check.h"

/* convering */
int covert(const char *desc, const char *src, char *input, size_t ilen, char *output, size_t olen) {
    char **pin = &input;
    char **pout = &output;
    iconv_t cd = iconv_open(desc, src);
    if (cd == (iconv_t)-1) {
        return -1;
    }
    memset(output, 0, olen);
    if (iconv(cd, pin, &ilen, pout, &olen)) return -1;
    iconv_close(cd);
    return 0;
}

/* code conver */
char *conver_code(char *input, const char *from_code, const char *to_code) {
    int flag;
    size_t len = strlen(input);
    size_t olen = LEN_512;

    char *output = (char *)malloc(olen);

    flag = covert(to_code, from_code, input, len, output, olen);
    if (flag == -1) {
        out_error("convert from %s to %s error!\n", from_code, to_code);
    }

    return output;
}
