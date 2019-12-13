#ifndef __CLI_FILE_H
#define __CLI_FILE_H

#include <stdint.h>

struct cli_list_t
{
    int fd;
    int32_t subscribe_mask;
    char name[20];
    struct cli_list_t *next;
};

extern struct cli_list_t *cli_head;

extern bool cli_list_is_empty();
extern void cli_list_add_entry(int);
extern void cli_list_unlink(struct cli_list_t *);
extern struct cli_list_t *cli_list_find_entry(int);
extern char *cli_list_get_name(int);
extern void cli_list_update_name(int, char *);
extern void cli_update_subscription_mask(struct cli_list_t *, uint8_t , bool);
#endif /* __CLI_FILE_H */