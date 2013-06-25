#pragma once

#include <unistd.h>
#include <stdexcept>

class reader
{
    int fd;
    char *buf;
    int avail;

public:
    reader(int fd, char *buf, int len) :
        fd(fd),
        buf(buf),
        avail(len)
    {}

    void read()
    {
        int res = ::read(fd, buf, avail);
        if (res < 0)
            throw std::runtime_error("Error while reading");
        avail -= res;
    }

};

class writer
{
    int fd;
    char *buf;
    int avail;

public:
    writer(int fd, char *buf, int len) :
        fd(fd),
        buf(buf),
        avail(len)
    {}

    void write()
    {
        int res = ::write(fd, buf, avail);
        if (res < 0)
            throw std::runtime_error("Error while writing");
        avail -= res;
    }

};
