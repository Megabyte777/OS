#pragma once

#include "epollfd.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <functional>
#include <algorithm>

class AOperation
{
protected:
    epollfd *efd;
    int fd;
    int operation;
    bool valid = true;

    AOperation(epollfd *efd, int fd, int operation) :
        efd(efd),
        fd(fd),
        operation(operation)
    {}

public:
    AOperation(AOperation &&x) :
        efd(x.efd),
        fd(x.fd),
        operation(x.operation),
        valid(x.valid)
    {
        x.valid = false;
    }

    ~AOperation()
    {
        if (valid)
            efd->unsubscribe(fd, operation);
    }

};

struct ARead : public AOperation
{
    template<typename reader_t>
    ARead(epollfd *efd, int fd, reader_t *reader, 
            std::function<void()> cont_ok, std::function<void()> cont_err) :
        AOperation(efd, fd, EPOLLIN)
    {
        efd->subscribe(fd, EPOLLIN,
                [this, &cont_ok, reader]()
                {
                    valid = false;
                    reader->read();
                    cont_ok();
                },
                [this, &cont_err]()
                {
                    valid = false;
                    cont_err();
                });
    }

};

struct AWrite : public AOperation
{
    template<typename writer_t>
    AWrite(epollfd *efd, int fd, writer_t *writer, 
            std::function<void()> cont_ok, std::function<void()> cont_err) :
        AOperation(efd, fd, EPOLLOUT)
    {
        efd->subscribe(fd, EPOLLOUT,
                [this, &cont_ok, writer]()
                {
                    valid = false;
                    writer->write();
                    cont_ok();
                },
                [this, &cont_err]()
                {
                    valid = false;
                    cont_err();
                });
    }
};

struct AAccept : public AOperation
{
    AAccept(epollfd *efd, int fd, sockaddr *addr, socklen_t *addrlen,
            std::function<void(int)> cont_ok, std::function<void()> cont_err) :
        AOperation(efd, fd, EPOLLIN)
    {
        efd->subscribe(fd, EPOLLIN,
                [this, fd, addr, addrlen, &cont_ok, &cont_err]()
                {
                    valid = false;
                    int client_fd = accept(fd, addr, addrlen);
                    if (client_fd > 0)
                        cont_ok(client_fd);
                    else
                        cont_err();
                },
                [this, &cont_err]()
                {
                    valid = false;
                    cont_err();
                });
    }
};
