#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <linux/in.h>
#include <unistd.h>
#include <strings.h>
#include <arpa/inet.h>

#define MAXREQ (4096 * 1024)

char buffer[MAXREQ], body[MAXREQ], msg[MAXREQ];

void error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Wrong number of arguments supplied. Should be 2, was %i.\n", argc - 1);
        return 1;
    }

    char *name = argv[1];
    int port = atoi(argv[2]);
    int socket_fd, new_socket_fd;
    socklen_t clilen;

    struct sockaddr_in serv_addr, cli_addr;
    int addrlen = sizeof(serv_addr);

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd == 0)
        error("socket failed");

    bzero((char *)&serv_addr, addrlen);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    if (bind(socket_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("bind failed");

    if (listen(socket_fd, 1) < 0)
        error("listen failed");

    if (new_socket_fd = accept(socket_fd, (struct sockaddr *)&serv_addr, (socklen_t *)&addrlen) < 0)
        error("accept failed");

    int readval = read(new_socket_fd, buffer, sizeof(buffer));
    printf("%s\n", buffer);
    return 0;
}