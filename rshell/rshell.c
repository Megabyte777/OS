#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define PORT "8822"
#define BACKLOG 5

int main()
{
    if (fork())
    {
        sleep(5);
        _exit(EXIT_SUCCESS);
    }
    setsid();
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    struct addrinfo *result;
    if (getaddrinfo(NULL, PORT, &hints, &result) != 0)
    {
        perror("GETADDRINFO");
        _exit(EXIT_FAILURE);
    }
    int sfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sfd == -1)
    {
        perror("SOCKET");
        _exit(EXIT_FAILURE);
    }
    int reuseaddrval = 1;
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &reuseaddrval, 
                sizeof(reuseaddrval)) != 0)
    {
        perror("SETSOCKOPT");
        _exit(EXIT_FAILURE);
    }
    if (bind(sfd, result->ai_addr, result->ai_addrlen) != 0)
    {
        perror("BIND");
        _exit(EXIT_FAILURE);
    }
    freeaddrinfo(result);
    if (listen(sfd, BACKLOG) != 0)
    {
        perror("LISTEN");
        _exit(EXIT_FAILURE);
    }
    struct sockaddr sa;
    socklen_t sl = sizeof(sa);
    while (1)
    {
        int fd = accept(sfd, &sa, &sl);
        if (fd == -1)
        {
            perror("ACCEPT");
            continue;
        }
        if (fork())
        {
            close(sfd);
            dup2(fd, 0);
            dup2(fd, 1);
            dup2(fd, 2);
            close(fd);
            write(1, "Hello\n", 6);
            _exit(EXIT_SUCCESS);
        }
        else
            close(fd);
    }
    return 0;
}
