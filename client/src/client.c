#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include "util.h"


/* Commands */
#define CMD_CONNECT "CONNECT"
#define CMD_DISCONNECT "DISCONNECT"
#define CMD_PUBLISH "PUBLISH"
#define CMD_SUBSCIBE "SUBSCRIBE"
#define CMD_UNSUBSCRIBE "UNSUBSCRIBE"

#define C(a, b) !strncasecmp((const char *)(a), (const char *)(b), sizeof (a) - 1)

typedef enum {
    CONNECT = 0,
    DISCONNECT,
    PUBLISH,
    SUBSCRIBE,
    UNSUBSCRIBE
} command_type_t;

struct cli_args {
    char client_name[20];
    size_t port;
    char topic[30];
    char data[512];
};

/* Static functions */
static void parse_server_message(char*);
static void split_server_message(char *);
static command_type_t cli_parse_command(char *, struct cli_args *);
static int cli_connect(size_t);
static void cli_send(int, char *, int);



int main()
{
    int socket_fd = -1;      /* Socket file descriptor */
    command_type_t cmd;
    struct cli_args cli_args;
    char buf[MAX_INPUT_LEN + 1] = {0};
    int len;
    fd_set readfds;
    int maxfd;
    int recv_count;

    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        if (socket_fd > 0)
        {
            FD_SET(socket_fd, &readfds);
        }
        maxfd = (socket_fd > STDIN_FILENO) ? socket_fd : STDIN_FILENO;
        select(maxfd + 1, &readfds, NULL, NULL, NULL);
        if (FD_ISSET(socket_fd, &readfds))
        {
            /* Message from server */
            memset(&buf, 0, sizeof buf);
            recv_count = read(socket_fd, buf, sizeof(buf) - 1);
            if (recv_count <= 0)
            {
                close(socket_fd);
                printf("Server closed connection...\n");
                break;
            }
            else
            {
                split_server_message(buf);
            }
        }
        if (FD_ISSET(STDIN_FILENO, &readfds))
        {
            /* User input */
            memset(&buf, 0, sizeof buf);
            read(STDIN_FILENO, buf, sizeof(buf) - 2); /* One char for terminate */
            terminate_command(buf);
            remove_trailing_chars(buf);
            cmd = cli_parse_command(buf, &cli_args);

            if (cmd == CONNECT)
            {
                socket_fd = cli_connect(cli_args.port);
                len = snprintf(buf, sizeof buf, "Client name:%s", cli_args.client_name);
                cli_send(socket_fd, buf, len);
            }
            else if (cmd == DISCONNECT)
            {
                shutdown(socket_fd, SHUT_WR);
                break;
            }
            else if (cmd == PUBLISH)
            {
                len = snprintf(buf, sizeof buf, "PUBLISH:%s:%s", cli_args.topic, cli_args.data);
                cli_send(socket_fd, buf, len);
            }
            else if (cmd == SUBSCRIBE)
            {
                len = snprintf(buf, sizeof buf, "SUBSCRIBE:%s", cli_args.topic);
                cli_send(socket_fd, buf, len);
            }
            else if (cmd == UNSUBSCRIBE)
            {
                len = snprintf(buf, sizeof buf, "UNSUBSCRIBE:%s", cli_args.topic);
                cli_send(socket_fd, buf, len);
            }
            else
            {
                printf("Unknown command...\n");
            }
        }
    }
    return 0;
}

static command_type_t
cli_parse_command(char *input, struct cli_args *cli_args)
{
    uint8_t args = 0;
    char *argv[5];
    char delim[] = ":";

    argv[args] = strtok(input, delim);
	while (argv[args] != NULL)
	{
		argv[++args] = strtok(NULL, delim);
	}
    if (C(CMD_CONNECT, argv[0]))
    {
        if (args != 3)
        {
            printf("Command CONNECT: Wrong number of parameters...\n");
            goto err;
        }
        cli_args->port = atoi(argv[1]);
        memmove(cli_args->client_name, argv[2], sizeof(cli_args->client_name) - 1);
        return CONNECT;
    }
    if (C(CMD_DISCONNECT, argv[0]))
    {
        if (args != 1)
        {
            printf("Command DISCONNECT: Wrong number of parameters...\n");
            goto err;
        }
        return DISCONNECT;
    }
    if (C(CMD_PUBLISH, argv[0]))
    {
        if (args != 3)
        {
            printf("Command PUBLISH: Wrong number of parameters...\n");
            goto err;
        }
        memmove(cli_args->topic, argv[1], sizeof(cli_args->topic) - 1);
        memmove(cli_args->data, argv[2], sizeof(cli_args->data) - 1);
        return PUBLISH;
    }
    if (C(CMD_SUBSCIBE, argv[0]))
    {
        if (args != 2)
        {
            printf("Command SUBSCRIBE: Wrong number of parameters...\n");
            goto err;
        }
        memmove(cli_args->topic, argv[1], sizeof(cli_args->topic) - 1);
        return SUBSCRIBE;
    }
    if (C(CMD_UNSUBSCRIBE, argv[0]))
    {
        if (args != 2)
        {
            printf("Command UNSUBSCRIBE: Wrong number of parameters...\n");
            goto err;
        }
        memmove(cli_args->topic, argv[1], sizeof(cli_args->topic) - 1);
        return UNSUBSCRIBE;
    }
    err:
    return -1;
}

static void
parse_server_message(char *buf)
{
    char *topic;
    char *data, *end;
    data = end = strchr(buf, ':');
    topic = buf;
    *end = '\0';
    if (++data)
    {
        printf("[Message] Topic:<%s> Data:<%s>\n", topic, data);
    }
}

static void
split_server_message(char *msg)
{
    char *input = NULL;
    char delimiter[2] = {0};
    remove_trailing_chars(msg);
    *delimiter = CMD_TERMINATE_CHAR;
    input = strtok(msg, delimiter);
    while (input != NULL)
    {
        parse_server_message(input);
        /* Get next input */
        input = strtok(NULL, delimiter);
    }
}

static int
cli_connect(size_t port)
{
    int socket_fd;
    struct sockaddr_in server_addr;    /* Server address */
    /* Create socket */
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1)
    {
        printf("Socket creation failed.\nBye,bye...\n");
        exit(1);
    }
    else
    {
       printf("Socket successfully created..\n");
    }
    memset(&server_addr, 0, sizeof(server_addr));
    // assign IP, PORT
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(port);

    // connect the client socket to server socket
    if (connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0)
    {
        printf("connection with the server failed.\nBye,bye...\n");
        exit(1);
    }
    else
    {
        printf("connected to the server..\n");
    }
    return socket_fd;
}

static void
cli_send(int fd, char *msg, int msg_len)
{
    if (fd <= 0)
    {
        return;
    }
    /* Send message */
    write(fd, msg, msg_len);
}

