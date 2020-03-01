#include <stdlib.h>
#include <string.h>
#include "util.h"

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

void
terminate_command(char *cmd)
{
    char *end = NULL;
    if ((end = strchr(cmd, '\r')))
    {
        *end = CMD_TERMINATE_CHAR;
        *(end + 1) = '\0';
    }
    if ((end = strchr(cmd, '\n')))
    {
        *end = CMD_TERMINATE_CHAR;
        *(end + 1) = '\0';
    }
}

