#include <stdlib.h>
#include <string.h>

void
remove_trailing_chars(char *str)
{
    char *end = NULL;
    if ((end = strchr(str, '\r')))
    {
        *end = '\0';
    }
    if ((end = strchr(str, '\n')))
    {
        *end = '\0';
    }
}