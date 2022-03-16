#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
//#include <linux/in.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include "bbuffer.h"
#include <pthread.h>

#define MAXREQ (4096 * 1024)

void error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

void *handle_request(void *bb) {
    // in child
    while (1)
    {
        char buffer[MAXREQ], body[MAXREQ], msg[MAXREQ];
        int bufferlen = sizeof(buffer);
        printf("preget\n");
        int fd = bb_get(bb);
        printf("postget\n");

        bzero(buffer, bufferlen);

        // TODO fix this
        if (read(fd, buffer, bufferlen - 1) < 0)
        {
            perror("read failed");
        }

        sleep(5);
        /*
        snprintf(body, sizeof(body),
                    "<html>\n<body>\n"
                    "<h1>Hello web browser</h1>\nYour request was\n"
                    "<pre>%s</pre>\n"
                    "</body>\n</html>\n",
                    buffer);
            snprint(msg, sizeof(msg),
                    "HTTP/0.9 200 OK\n"
                    "Content-Type: text/html\n"
                    "Content-Length: %d\n\n%s",
                    strlen(body), body);
            */
        write(fd, buffer, strlen(buffer));

        close(fd);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 5)
    {
    printf("Wrong number of arguments supplied. Should be 2, was %i.\n", argc - 1);
        return 1;
    }

    char *name = argv[1];
    int port = atoi(argv[2]);
    int thread_count = atoi(argv[3]);
    int buffer_size = atoi(argv[4]);

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
    // printf("%i\n", pid);
        // in parent
    while (1)
    {
        clilen = sizeof(cli_addr);
        if ((new_socket_fd = accept(socket_fd, (struct sockaddr *)&cli_addr, (socklen_t *)&clilen)) < 0)
            error("accept failed");
        printf("preadd\n");
        bb_add(bb, new_socket_fd);
        printf("postadd\n");
    }
}
