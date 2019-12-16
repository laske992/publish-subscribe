#ifndef __SUBSCRIBE_FILE_H
#define __SUBSCRIBE_FILE_H

#include "cli.h"

extern void publish_all_subscribers(int, char *, char *);
extern void subscribe_handle(int, char *);
extern void unsubscribe_handle(int, char *);
extern void subscribe_list_unlink_entry(struct cli_list_t *);

#endif /* __SUBSCRIBE_FILE_H */
