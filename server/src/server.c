#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include "util.h"
#include "list.h"
#include "cli.h"
#include "subscribe.h"

extern struct cli_list_t *cli_head;

/* Static functions */
static int server_setup_tcp_socket(int, struct sockaddr_in *, int);
static int server_accept_connection(int, struct sockaddr_in *, int);
static void server_close_connection(struct cli_list_t *, struct sockaddr_in *, int);
static void server_parse_recv_msg(int, char *);
static void server_publish_handle(int, char *);
static void server_set_fd_flags(int, fd_set *, int *);


int main(int argc, char **argv)
{
    int socket_fd; /* Socket file descriptor */
    int cli_fd, len;
    int port; /* Server listening port */
    struct cli_list_t *cli;
    struct sockaddr_in server_addr; /* Server address */
    struct sockaddr_in cli_addr;
    int nfds;
    int activity;
    int recv_count;      /* Value received from socket */
    char recv_msg[1024]; /* Message received from socket */
    fd_set readfds;

    /* Check input arguments */
    if (argc != 2)
    {
        printf("Wrong number of input parameters.\nBye, bye...\n");
        exit(1);
    }
    /* Parse port */
    port = atoi(argv[1]);
    if (!port)
    {
        printf("Port not valid.\nBye, bye...\n");
        exit(1);
    }
    len = sizeof(struct sockaddr_in);
    /* Zero structures */
    memset(&server_addr, 0, len);
    memset(&cli_addr, 0, len);

    /* Create & setup TCP/IP socket */
    socket_fd = server_setup_tcp_socket(port, &server_addr, len);

    while (1)
    {
        server_set_fd_flags(socket_fd, &readfds, &nfds);
        activity = select(nfds + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR))
        {
            printf("select error");
        }

        /* Incoming connection */
        if (FD_ISSET(socket_fd, &readfds))
        {
            cli_fd = server_accept_connection(socket_fd, &cli_addr, len);
            /* save client to the list */
            cli_list_add_entry(cli_fd);
        }
        /* Check if any client is active */
        if (cli_list_is_empty())
        {
            continue; /* No clients in the list */
        }
        list_for_each(cli_head, cli)
        {
            /* IO operation on some other socket */
            if (FD_ISSET(cli->fd, &readfds))
            {
                memset(&recv_msg, 0, sizeof recv_msg);
                recv_count = read(cli->fd, recv_msg, sizeof(recv_msg) - 1);
                if (recv_count == 0)
                {
                    /* Client disconnected */
                    server_close_connection(cli, &cli_addr, len);
                }
                else
                {
                    /* Handle received message */
                    remove_trailing_chars(recv_msg);
                    server_parse_recv_msg(cli->fd, recv_msg);
                }
            }
        }
        if (FD_ISSET(STDIN_FILENO, &readfds))
        {
            memset(&recv_msg, 0, sizeof recv_msg);
            recv_count = read(STDIN_FILENO, recv_msg, sizeof(recv_msg) - 1);
            if (!strncmp(recv_msg, "exit", recv_count - 1))
            {
                break;
            }
        }
    }
    close(socket_fd);
}

static int
server_setup_tcp_socket(int port, struct sockaddr_in *addr, int addr_len)
{
    int socket_fd;
    /* Create TCP/IP socket */
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1)
    {
        printf("Failed to create socket.\nBye, bye...\n");
        exit(1);
    }
    else
    {
        printf("Socket successfully created...\n");
    }

    /* Initialize server address structre */
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = htonl(INADDR_ANY);
    addr->sin_port = htons(port);

    /* Bind socket to given IP and verification */
    if ((bind(socket_fd, (struct sockaddr *)addr, addr_len)) != 0)
    {
        printf("Socket bind failed...\nBye,bye...");
        exit(1);
    }
    else
    {
        printf("Socket successfully binded...\n");
    }

    if ((listen(socket_fd, 20)) != 0)
    {
        printf("Listen failed...\nBye,bye...");
        exit(1);
    }
    else
    {
        printf("Server listening..\n");
    }
    return socket_fd;
}

static int
server_accept_connection(int socket_fd, struct sockaddr_in *cli_addr, int len)
{
    int cli_fd;
    cli_fd = accept(socket_fd, (struct sockaddr *)cli_addr, (socklen_t *)&len);
    if (cli_fd < 0)
    {
        return cli_fd;
    }
    printf("[INFO]New client connected: fd: %d, ip: %s, port: %d, ",
           cli_fd, inet_ntoa(cli_addr->sin_addr), ntohs(cli_addr->sin_port));
    return cli_fd;
}

static void
server_close_connection(struct cli_list_t *cli, struct sockaddr_in *cli_addr, int len)
{
    getpeername(cli->fd, (struct sockaddr *)cli_addr, (socklen_t *)&len);
    printf("[INFO]Client disconnected: ip: %s, port: %d, name: %s \n",
           inet_ntoa(cli_addr->sin_addr), ntohs(cli_addr->sin_port), cli->name);
    /* Close the socket and remove client from the list */
    close(cli->fd);
    subscribe_list_unlink_entry(cli);
    cli_list_unlink(cli);
}

static void
server_parse_recv_msg(int fd, char *msg)
{
    char *end;
    char *ptr;
    int len;
    end = strchr(msg, ':');

    if (!end)
    {
        printf("Wrong input message!\n");
        return;
    }
    len = end - msg - 1;
    ptr = ++end;
    if (!(strncmp(msg, "Client name", len)))
    {
        cli_list_update_name(fd, ptr);
        printf("name: %s\n", ptr);
    }
    else if (!(strncmp(msg, "PUBLISH", len)))
    {
        server_publish_handle(fd, ptr);
    }
    else if (!(strncmp(msg, "SUBSCRIBE", len)))
    {
        subscribe_handle(fd, ptr);
    }
    else if (!(strncmp(msg, "UNSUBSCRIBE", len)))
    {
        unsubscribe_handle(fd, ptr);
    }
}

static void
server_publish_handle(int fd, char *publish_data)
{
    char *end;
    char *data;
    int len;
    char topic[30] = {0};

    end = strchr(publish_data, ':');
    if (!end)
    {
        printf("Wrong publish message!\n");
        return;
    }
    len = end - publish_data;
    data = ++end;
    memmove(topic, publish_data, len);
    printf("%s published: <%s>:<%s>\n", cli_list_get_name(fd), topic, data);
    publish_all_subscribers(fd, topic, data);
}

static void
server_set_fd_flags(int fd, fd_set *readfds, int *nfds)
{
    struct cli_list_t *entry;
    FD_ZERO(readfds);
    FD_SET(fd, readfds);
    FD_SET(STDIN_FILENO, readfds);

    *nfds = fd;
    if (!cli_list_is_empty())
    {
        list_for_each(cli_head, entry)
        {
            if (entry->fd > 0)
            {
                FD_SET(entry->fd, readfds);
            }
            if (entry->fd > *nfds)
            {
                *nfds = entry->fd;
            }
        }
    }
}
