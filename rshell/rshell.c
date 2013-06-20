#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <pty.h>
#include <errno.h>
#include <poll.h>

#define PORT "8822"
#define BACKLOG 5
#define POLL_TAIL_FLAGS POLLERR | POLLHUP | POLLNVAL

int write_(int fd, char *buf, size_t count)
{
    size_t printed = 0;
    while (printed < count)
    {
        int res = write(fd, buf + printed, count - printed);
        if (res < 0)
        {
            perror("WRITE");
            return -1;
        }
        printed += res;
    }
    return 0;
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
        perror("Connection accepted");
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
                struct pollfd pfd[2];
                pfd[0].fd = fd;
                pfd[0].events = POLLIN | POLL_TAIL_FLAGS;
                pfd[1].fd = master;
                pfd[1].events = POLLIN | POLL_TAIL_FLAGS;
                const size_t BUF_SIZE = 4096;
                char fd2master[BUF_SIZE];
                size_t fd2master_s = 0;
                char master2fd[BUF_SIZE];
                size_t master2fd_s = 0;
                int dead_fd = 0;
                int dead_master = 0;
                while (1)
                {
                    if ((dead_fd && dead_master) || (dead_fd && fd2master_s == 0) 
                            || (dead_master && master2fd_s == 0))
                        break;
                    if (poll(pfd, 2, -1) == -1)
                    {
                        if (errno == EINTR)
                            continue;
                        else
                            break;
                    }
                    if (pfd[0].revents & (POLL_TAIL_FLAGS))
                        dead_fd = 1;
                    if (pfd[1].revents & (POLL_TAIL_FLAGS))
                        dead_master = 1;
                    if ((pfd[0].revents & POLLIN) &&
                            fd2master_s < BUF_SIZE)
                    {
                        int res = read(fd, 
                                fd2master + fd2master_s, BUF_SIZE - fd2master_s);
                        if (res <= 0)
                            pfd[0].events ^= POLLIN;
                        else
                            fd2master_s += res;
                    }
                    if ((pfd[1].revents & POLLIN) &&
                            master2fd_s < BUF_SIZE)
                    {
                        int res = read(master, 
                                master2fd + master2fd_s, BUF_SIZE - master2fd_s);
                        if (res <= 0)
                            pfd[1].events ^= POLLIN;
                        else
                            master2fd_s += res;
                    }

                    if (pfd[0].revents & POLLOUT)
                    {
                        int res = write_(fd, master2fd, master2fd_s);
                        if (res < 0)
                            pfd[0].events ^= POLLOUT;
                        master2fd_s = 0;
                        pfd[0].events ^= POLLOUT;
                    }
                    if (pfd[1].revents & POLLOUT)
                    {
                        if (fd2master[fd2master_s - 1] == '\n')
                            fd2master_s--;
                        int res = write_(master, fd2master, fd2master_s);
                        if (res < 0)
                            pfd[1].events ^= POLLOUT;
                        fd2master_s = 0;
                        pfd[1].events ^= POLLOUT;
                    }
                    if (master2fd_s > 0)
                        pfd[0].events |= POLLOUT;
                    if (fd2master_s > 0)
                        pfd[1].events |= POLLOUT;
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
