#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "cli.h"
#include "util.h"
#include "list.h"


struct cli_list_t *cli_head = NULL;
uint8_t cli_count;

/* Static functions */
static void cli_inc_count();
static void cli_dec_count();

/*
 * @brief: Check if list is empty
 * @return: true if empty
*/
bool
cli_list_is_empty(void)
{
    return list_is_empty(cli_head);
}

/*
 * @brief: Save clinet to the list.
 * @param fd: client socket file descriptor
*/
void
cli_list_add_entry(int fd)
{
    struct cli_list_t *entry;
    struct cli_list_t *pos;
    entry = (struct cli_list_t *)malloc(sizeof(struct cli_list_t));
    memset(entry, 0, sizeof(struct cli_list_t));
    entry->fd = fd;
    list_add(cli_head, entry, pos);
    cli_inc_count();
}

/*
 * @brief: Unlink entry from the client list
*/
void
cli_list_unlink(struct cli_list_t *cli)
{
    struct cli_list_t *prev;
    list_unlink(cli_head, cli, prev);
    cli_dec_count();
    free(cli);
}

/*
 * @brief: Find certain client in the client list
 * @param fd: file descriptor to find
 * @return: NULL or found client
*/
struct cli_list_t *
cli_list_find_entry(int fd)
{
    struct cli_list_t *cli;
    list_for_each(cli_head, cli)
    {
        if (cli->fd == fd)
        {
            break;  /* Found */
        }
    }
    return cli;
}

struct cli_list_t *
cli_get_next(struct cli_list_t *cli)
{
    if (!cli)
    {
        return cli_head;
    }
    return cli->next;
}

char *
cli_list_get_name(int fd)
{
    struct cli_list_t *cli = cli_list_find_entry(fd);
    if (!cli)
    {
        return NULL;
    }
    return cli->name;
}

/*
 * @brief: Save client name
 * @param fd: socket file descriptor
 * @param name: clinet name
*/
void
cli_list_update_name(int fd, char *name)
{
    struct cli_list_t *cli = cli_list_find_entry(fd);
    if (!cli)
    {
        return;
    }
    memmove(cli->name, name, sizeof cli->name);
}

void
cli_update_subscription_mask(struct cli_list_t *cli, uint8_t topic_id, bool subscribe)
{
    if (!cli)
    {
        return;     /* Error */
    }
    if (subscribe)
    {
        SET_BIT(cli->subscribe_mask, topic_id);
    }
    else
    {
        CLEAR_BIT(cli->subscribe_mask, topic_id);
    }
}

/* Get current number of clients */
uint8_t
cli_get_count(void)
{
    return cli_count;
}

/* Increment number of clients */
static void
cli_inc_count(void)
{
    cli_count++;
}

/* Decrement number of clients */
static void
cli_dec_count(void)
{
    if (cli_get_count())
    {
        cli_count--;
    }
}
