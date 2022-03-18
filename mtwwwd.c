#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include "bbuffer.h"
#include <pthread.h>
#include <linux/limits.h>

#define MAXREQ (4096 * 1024)

char *webroot;

void error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

void *handle_request(void *bb)
{
    while (1)
    {
        char buffer[MAXREQ];
        int bufferlen = sizeof(buffer);
        int fd = bb_get(bb);

        bzero(buffer, bufferlen);

        if (read(fd, buffer, bufferlen - 1) < 0)
            error("read failed");

        char *urlstart = strstr(buffer, "/");
        char *urlend = strstr(urlstart, " ");
        size_t urllen = urlend - urlstart;
        char *url = (char *)malloc(urllen);
        strncpy(url, urlstart, urllen);

        char root[sizeof(webroot) + urllen];
        strcpy(root, webroot);
        strcat(root, url);
        free(url);

        char path[PATH_MAX];
        realpath(root, path);

        // TODO verify path

        FILE *fp = fopen(path, "r");

        fseek(fp, 0L, SEEK_END);
        int filesize = ftell(fp) + 1;
        rewind(fp);

        snprint(buffer, "HTTP/0.9 200 OK\n"
                        "Content-Type: text/html\n"
                        "Content-Length: %d\n\n",
                filesize);
        write(fd, buffer, strlen(buffer));

        while (fread(buffer, sizeof(char), MAXREQ, fp))
        {
            write(fd, buffer, strlen(buffer));
            printf("Writing\n");
        }

        close(fd);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 5)
        error("Wrong number of arguments supplied. Should be 4");

    webroot = argv[1];
    int port = atoi(argv[2]);
    int thread_count = atoi(argv[3]);
    int buffer_size = atoi(argv[4]);
    int opt = 1;

    int socket_fd, new_socket_fd;
    socklen_t clilen;

    struct sockaddr_in serv_addr, cli_addr;
    int addrlen = sizeof(serv_addr);

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    bzero((char *)&serv_addr, addrlen);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    if (socket_fd == 0)
        error("socket failed");

    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("Failed to set socket options");
    }

    if (bind(socket_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("bind failed");

    if (listen(socket_fd, 1) < 0)
        error("listen failed");

    pid_t pids[thread_count];

    BNDBUF *bb = bb_init(buffer_size);

    pthread_t threads[thread_count];

    for (int i = 0; i < thread_count; i++)
    {
        pthread_create(&threads[i], NULL, &handle_request, bb);
    }
    while (1)
    {
        clilen = sizeof(cli_addr);
        if ((new_socket_fd = accept(socket_fd, (struct sockaddr *)&cli_addr, (socklen_t *)&clilen)) < 0)
            error("accept failed");
        bb_add(bb, new_socket_fd);
    }
}
