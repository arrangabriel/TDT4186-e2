#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <linux/in.h>
#include <unistd.h>

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
    int server_fd, new_socket;
    char buffer[1024] = {0};

    struct sockaddr_in address;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == 0)
        error("socket failed");

    address.sin_family = AF_INET;
    // What is this? figure it out
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = port;

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
        error("bind failed");

    if (listen(server_fd, 1) < 0)
        error("listen failed");

    if (new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen) < 0)
        error("accept failed");

    int readval = read(new_socket, buffer, sizeof(buffer));
    printf("%s\n", buffer);
    return 0;
}