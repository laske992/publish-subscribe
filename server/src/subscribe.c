#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include "util.h"
#include "list.h"
#include "cli.h"


struct subscribe_list_t
{
    struct cli_list_t *cli;
    struct subscribe_list_t *next;
};

char topic_array[MAX_TOPIC][MAX_TOPIC_LEN + 1];
struct subscribe_list_t *subs_list_head[MAX_TOPIC];

/* Static functions */
static struct subscribe_list_t *subscribe_find_entry(uint8_t, struct cli_list_t *);
static void subscribe_client(struct cli_list_t *, uint8_t);
static void list_unlink_by_cli(uint8_t, struct cli_list_t *);
static uint8_t save_topic(char *);
static uint8_t get_topic_id(char *);
static uint8_t get_vacant_topic_id();
static void forget_topic(uint8_t);

/*
 * @brief: Publish to all subscribers
 * @param fd: topic to subscribe
 * @param topic: message to publish
 * @param msg_len: message length
*/
void
publish_all_subscribers(int publisher, char *topic, char *msg)
{
    uint8_t topic_id;
    struct subscribe_list_t *entry = NULL;
    char data[MAX_INPUT_LEN + 1] = {0};
    topic_id = get_topic_id(topic);
    snprintf(data, sizeof data, "%s:%s", topic, msg);
    list_for_each(subs_list_head[topic_id], entry)
    {
        if (entry->cli->fd == publisher)
        {
            continue;   /* Do not self-publish */
        }
        printf(">%s informed: %s\n", entry->cli->name, data);
        send(entry->cli->fd, data, strlen(data), 0);        
    }
}

/*
 * @brief: Client to be subsribed to certain topic
 * @param fd: clinet socket file descriptor
 * @param topic: wanted topic
*/
void
subscribe_handle(int fd, char *topic)
{
    uint8_t topic_id;
    struct cli_list_t *cli = cli_list_find_entry(fd);    
    if (!cli)
    {
        return;
    }
    topic_id = save_topic(topic);
    if (topic_id >= MAX_TOPIC)
    {
        printf("Maximum (32) topics reached...discarding %s's request\n", cli->name);
        return;
    }
    subscribe_client(cli, topic_id);
    cli_update_subscription_mask(cli, topic_id, true);
    printf("%s subscribed to: <%s>\n", cli->name, topic);
}

/*
 * @brief: Forget certain topic for specific client
 * @param fd: clinet socket file descriptor
 * @param topic: wanted topic
*/
void
unsubscribe_handle(int fd, char *topic)
{
    uint8_t topic_id;
    struct subscribe_list_t *entry, *prev;
    topic_id = get_topic_id(topic);
    if (topic_id >= MAX_TOPIC)
    {
        return; /* Client is not subscribe to this topic */
    }
    list_for_each(subs_list_head[topic_id], entry)
    {
        if (entry->cli->fd == fd)
        {
            cli_update_subscription_mask(entry->cli, topic_id, false);
            list_unlink(subs_list_head[topic_id], entry, prev);
            printf("%s unsubscribed from: <%s>\n", entry->cli->name, topic);
        }
    }
    if (list_is_empty(subs_list_head[topic_id]))
    {
        forget_topic(topic_id);
    }
}

/*
 * @brief: Unlink entry from the subscription list.
 *          Update topic array.
*/
void
subscribe_list_unlink_entry(struct cli_list_t *cli)
{
    uint8_t i;
    for (i = 0; i < MAX_TOPIC; i++)
    {
        if (IS_BIT_SET(cli->subscribe_mask, i))
        {
            list_unlink_by_cli(i, cli);
            if (list_is_empty(subs_list_head[i]))
            {
                forget_topic(i);
            }
        }
    }
}

/*
 * @brief: Find certain entry in the subsciption list
 * @param cli: client to find
 * @return: NULL or found entry
*/
static struct subscribe_list_t *
subscribe_find_entry(uint8_t id, struct cli_list_t *cli)
{
    struct subscribe_list_t *entry;
    list_for_each(subs_list_head[id], entry)
    {
        if (entry->cli == cli)
        {
            break;  /* Found */
        }
    }
    return entry;
}

static void
subscribe_client(struct cli_list_t *cli, uint8_t topic_id)
{
    struct subscribe_list_t *entry;
    struct subscribe_list_t *pos;

    entry = malloc(sizeof(struct subscribe_list_t));
    memset(entry, 0, sizeof(struct subscribe_list_t));
    entry->cli = cli;
    list_add(subs_list_head[topic_id], entry, pos);
}

static void
list_unlink_by_cli(uint8_t topic_id, struct cli_list_t *cli)
{
    struct subscribe_list_t *prev;
    struct subscribe_list_t *entry;

    if (list_is_empty(subs_list_head[topic_id]))
    {
        return; /* Nothing to do */
    }
    entry = subscribe_find_entry(topic_id, cli);
    list_unlink(subs_list_head[topic_id], entry, prev);
    free(entry);
}

static uint8_t
save_topic(char *topic)
{
    uint8_t topic_id;
    uint8_t len;
    uint8_t vacant_topic_id;

    topic_id = get_topic_id(topic);
    if (topic_id < MAX_TOPIC)
    {
        return topic_id;    /* Topic already exits */
    }
    vacant_topic_id = get_vacant_topic_id();
    if (vacant_topic_id > 31)
    {
        return MAX_TOPIC;
    }
    len = sizeof(topic_array[0]) - 1;
    strncpy(topic_array[vacant_topic_id], topic, len);
    *(topic_array[vacant_topic_id] + len) = '\0';
    return vacant_topic_id;
}

static uint8_t
get_topic_id(char *topic)
{
    uint8_t i;
    for (i = 0; i < MAX_TOPIC; i++)
    {
        if (strncmp(topic_array[i], topic, sizeof(topic_array[i])))
        {
            continue;   /* Continue searching... */
        }
        else
        {
            break;  /* Topic id found */
        }
    }
    return i;
}

static uint8_t
get_vacant_topic_id(void)
{
    uint8_t i;
    for (i = 0; i < MAX_TOPIC; i++)
    {
        if (!topic_array[i][0])
        {
            break;  /* Topic id found */
        }
    }
    return i;
}

static void
forget_topic(uint8_t topic_id)
{
    memset(topic_array[topic_id], 0, sizeof(topic_array[0]));
}
