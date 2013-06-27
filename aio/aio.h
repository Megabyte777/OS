#pragma once

#include "epollfd.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <functional>
#include <list>

class AOperation
{
protected:
    epollfd *efd;
    int fd;
    int operation;
    bool *valid;

    AOperation(epollfd *efd, int fd, int operation) :
        efd(efd),
        fd(fd),
        operation(operation)
    {
        valid = new bool(true);
    }

public:

    AOperation()
    {
        valid = new bool(false);
    }

    AOperation(AOperation &&x) :
        efd(x.efd),
        fd(x.fd),
        operation(x.operation),
        valid(x.valid)
    {
        x.valid = new bool(false);
    }

    AOperation& operator=(AOperation &&rhs)
    {
        efd = rhs.efd;
        fd = rhs.fd;
        operation = rhs.operation;
        delete valid;
        valid = rhs.valid;
        rhs.valid = new bool(false);
        return *this;
    }

    ~AOperation()
    {
        if (*valid)
            efd->unsubscribe(fd, operation);
        delete valid;
    }

};

struct ARead : public AOperation
{
    template<typename reader_t>
    ARead(epollfd *efd, int fd, reader_t *reader, 
            std::function<void()> cont_ok, std::function<void()> cont_err) :
        AOperation(efd, fd, EPOLLIN)
    {
        bool *valid_(valid);
        efd->subscribe(fd, EPOLLIN,
                [valid_, cont_ok, reader]()
                {
                    *valid_ = false;
                    reader->read();
                    cont_ok();
                },
                [valid_, cont_err]()
                {
                    *valid_ = false;
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
        bool *valid_(valid);
        efd->subscribe(fd, EPOLLOUT,
                [valid_, cont_ok, writer]()
                {
                    *valid_ = false;
                    writer->write();
                    cont_ok();
                },
                [valid_, cont_err]()
                {
                    *valid_ = false;
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
        bool *valid_(valid);
        efd->subscribe(fd, EPOLLIN,
                [valid_, fd, addr, addrlen, cont_ok, cont_err]()
                {
                    *valid_ = false;
                    int client_fd = accept(fd, addr, addrlen);
                    if (client_fd > 0)
                        cont_ok(client_fd);
                    else
                        cont_err();
                },
                [valid_, cont_err]()
                {
                    *valid_ = false;
                    cont_err();
                });
    }
};

class aio_manager
{
    std::list<AOperation> operations;
    epollfd *efd;

    std::list<AOperation>::iterator reserve_place()
    {
        operations.push_back(std::move(AOperation()));
        return --operations.end();
    }

public:

    aio_manager(epollfd *efd) :
        efd(efd)
    {}

    template<typename reader_t>
    void read(int fd, reader_t *reader, 
            std::function<void()> cont_ok, std::function<void()> cont_err)
    {
        auto place = reserve_place();
        *place = ARead(efd, fd, reader, 
                [this, cont_ok, place]()
                {
                    operations.erase(place);
                    cont_ok();
                },
                [this, cont_err, place]()
                {
                    operations.erase(place);
                    cont_err();
                });
    }

    template<typename writer_t>
    void write(int fd, writer_t *writer, 
            std::function<void()> cont_ok, std::function<void()> cont_err)
    {
        auto place = reserve_place();
        *place = AWrite(efd, fd, writer, 
                [this, cont_ok, place]()
                {
                    operations.erase(place);
                    cont_ok();
                },
                [this, cont_err, place]()
                {
                    operations.erase(place);
                    cont_err();
                });
    }
    
    void accept(epollfd *efd, int fd, sockaddr *addr, socklen_t *addrlen,
            std::function<void(int)> cont_ok, std::function<void()> cont_err)
    {
        auto place = reserve_place();
        *place = AAccept(efd, fd, addr, addrlen, 
                [this, cont_ok, place](int client_fd)
                {
                    operations.erase(place);
                    cont_ok(client_fd);
                },
                [this, cont_err, place]()
                {
                    operations.erase(place);
                    cont_err();
                });
    }

    void cycle()
    {
        efd->cycle();
    }

};
