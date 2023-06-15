#include <ctype.h>
#include <string.h>
#include <utils/stringUtils.h>

void convertToUpper(char *str) {
    int length = strlen(str);
    for (int i = 0; i < length; i++) {
        str[i] = toupper(str[i]);
    }
}
