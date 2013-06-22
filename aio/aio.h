#pragma once

#include "epollfd.h"
#include <functional>
#include <algorithm>

template<typename reader_t>
class ARead
{
    epollfd *efd;
    int fd;
    reader_t *reader;
    bool valid = true;

    ARead() = delete;

public:
    ARead(epollfd *efd, int fd, reader_t *reader, 
            std::function<void()> cont_ok, std::function<void()> cont_err) :
        efd(efd),
        fd(fd),
        reader(reader)
    {
        efd->subscribe(fd, EPOLLIN,
                [this, &cont_ok]()
                {
                    valid = false;
                    this->reader->read();
                    cont_ok();
                },
                [this, &cont_err]()
                {
                    valid = false;
                    cont_err();
                });
    }

    ARead(ARead &&x)
    {
        efd = x.efd;
        fd = x.fd;
        reader = x.reader;
        valid = x.valid;
    }

    ~ARead()
    {
        if (valid)
            efd->unsubscribe(fd, EPOLLIN);
    }

};

template<typename writer_t>
class AWrite
{
    epollfd *efd;
    int fd;
    writer_t *writer;
    bool valid = true;

    AWrite() = delete;

public:
    AWrite(epollfd *efd, int fd, writer_t *writer, 
            std::function<void()> cont_ok, std::function<void()> cont_err) :
        efd(efd),
        fd(fd),
        writer(writer)
    {
        efd->subscribe(fd, EPOLLOUT,
                [this, &cont_ok]()
                {
                    valid = false;
                    this->writer->write();
                    cont_ok();
                },
                [this, &cont_err]()
                {
                    valid = false;
                    cont_err();
                });
    }

    AWrite(AWrite &&x)
    {
        efd = x.efd;
        fd = x.fd;
        writer = x.writer;
        valid = x.valid;
    }

    ~AWrite()
    {
        if (valid)
            efd->unsubscribe(fd, EPOLLOUT);
    }

};

