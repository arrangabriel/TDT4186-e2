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
#include <sys/stat.h>
#include <sys/types.h>

#define MAXREQ (4096 * 1024)
char webroot[PATH_MAX];

void error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int path_contains(char *path, char *subpath)
{
    return strstr(path, subpath) != NULL;
}

void *handle_request(void *bb)
{
    while (1)
    {
        char buffer[MAXREQ];
        int bufferlen = sizeof(buffer);
        int urllen = PATH_MAX - strlen(webroot);
        int fd = bb_get(bb);

        bzero(buffer, bufferlen);

        if (read(fd, buffer, 4 + urllen) < 0)
            error("read failed");

        char req_type[3];
        char url[urllen];
        sscanf(buffer, "%3s %s", req_type, url);
        if (strstr(buffer, " ") - &buffer[0] != 3 || strcmp(req_type, "GET"))
        {
            char notfound[] = "HTTP/0.9 400 Bad Request";
            write(fd, notfound, strlen(notfound));
            close(fd);
            continue;
        }

        /*
         * The program is configured to parse requests on the form '(3 character string)(whitespace)(document-path)(whitespace/null character/newline)(arbitrary characters)'
         * This is in line with the HTTP 0.9 protocol, which has the form 'GET /path'.
         * Note that any deviation from this form will result in a Bad Request response.
         *
         */
        char real_path[PATH_MAX];
        char path[PATH_MAX];
        strcpy(path, webroot);
        strcat(path, url);
        realpath(path, real_path);
        /****************************************************************************************************\
         * EXPLOIT SOLUTIONS                                                                                *
         * By using .. a client can access files on the entire file system where the server is running.     *
         *                                                                                                  *
         * This if-statement encompasses our two solutions to the exploit.                                  *
         *                                                                                                  *
         * The first verify_path check is the most sophisticated solution.                                  *
         * First we translate both the full requested path (path),                                          *
         * and the root path (root_path) to their respective canonical forms.                               *
         * We then check to se if the root path is contained within the canonical form of the request path. *
         *                                                                                                  *
         * The second verify_path call simply checks if the request url contains (..).                      *
         \***************************************************************************************************/
        if (!path_contains(real_path, webroot) || path_contains(url, ".."))
        {
            char forbidden[] = "HTTP/0.9 403 Forbidden";
            write(fd, forbidden, strlen(forbidden));
            close(fd);
            continue;
        }

        FILE *fp = fopen(path, "r");

        struct stat filetype;
        stat(path, &filetype);

        if (fp == NULL || !S_ISREG(filetype.st_mode))
        {
            char notfound[] = "HTTP/0.9 404 Not Found";
            write(fd, notfound, strlen(notfound));
            close(fd);
            continue;
        }

        fseek(fp, 0L, SEEK_END);
        int filesize = ftell(fp) + 1;
        rewind(fp);

        while (fread(buffer, sizeof(char), MAXREQ, fp))
        {
            write(fd, buffer, strlen(buffer));
        }

        close(fd);
    }
}

int main(int argc, char *argv[])
{
    char default_path[PATH_MAX];
    realpath("./documents", default_path);
    if (argc != 5)
        error("Wrong number of arguments supplied. Required arguments are: www-path port #threads #bufferslots.");

    if (strcmp(argv[1], "default") == 0)
        strcpy(webroot, default_path);
    else
        realpath(argv[1], webroot);
    
    int port = atoi(argv[2]);
    int thread_count = atoi(argv[3]);
    int buffer_size = atoi(argv[4]);
    int opt = 1;

    int socket_fd, new_socket_fd;
    socklen_t clilen;

    struct sockaddr_in6 serv_addr, cli_addr;
    int addrlen = sizeof(serv_addr);

    socket_fd = socket(AF_INET6, SOCK_STREAM, 0);

    bzero((char *)&serv_addr, addrlen);

    serv_addr.sin6_family = AF_INET6;
    serv_addr.sin6_addr = in6addr_any;
    serv_addr.sin6_port = htons(port);

    if (socket_fd == 0)
        error("Socket creation failed");

    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
        error("Failed to set socket options");

    if (bind(socket_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("Socket bind failed");

    if (listen(socket_fd, 1) < 0)
        error("Socket listen failed");

    BNDBUF *bb = bb_init(buffer_size);

    pthread_t threads[thread_count];

    for (int i = 0; i < thread_count; i++)
        pthread_create(&threads[i], NULL, &handle_request, bb);

    while (1)
    {
        clilen = sizeof(cli_addr);
        if ((new_socket_fd = accept(socket_fd, (struct sockaddr *)&cli_addr, (socklen_t *)&clilen)) < 0)
            error("accept failed");
        bb_add(bb, new_socket_fd);
    }
}