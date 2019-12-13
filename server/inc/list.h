#ifndef __LIST_FILE_H
#define __LIST_FILE_H

#include <stdbool.h>
#include <stdlib.h>


#define list_for_each(head, pos)     \
    for ((pos) = (head); (pos); (pos) = (pos)->next)

#define list_get_last(head, pos)     \
    for ((pos) = (head); (pos)->next; (pos) = (pos)->next);

#define list_add(head, entry, last)          \
    if (list_is_empty((head))) {             \
        (head) = (entry);                    \
    } else {                                 \
        list_get_last((head), (last));       \
        (last)->next = (entry);              \
    }

#define  list_unlink(head, entry, prev)      \
    if ((head) == (entry)) {                 \
        (head) = (entry)->next;              \
    } else {                                 \
        list_for_each((head), (prev)) {      \
            if ((prev)->next == (entry)) {   \
                (prev)->next = (entry)->next;\
                break;                       \
            }                                \
        }                                    \
    }

extern bool list_is_empty(void *);
#endif /* __LIST_FILE_H */
