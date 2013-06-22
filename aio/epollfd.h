#pragma once

#include <sys/epoll.h>
#include <unistd.h>
#include <functional>
#include <map>
#include <stdexcept>

class epollfd
{
    std::map< std::pair<int, int>,
        std::pair<std::function<void()>, std::function<void()> > > queue;
    std::map<int, int> modes;
    int epollfd_;
    int count;

public:
    epollfd() :
        count(0)
    {
        epollfd_ = epoll_create1(0);
        if (epollfd_ == -1)
            throw std::runtime_error("Error while initializing");
    }

    ~epollfd()
    {
        close(epollfd_);
    }

    void subscribe(int fd, int what,
            std::function<void()> cont_ok, std::function<void()> cont_err)
    {
        if (queue.count(std::make_pair(fd, what)))
            throw std::runtime_error("Such queue element already exists");
        queue[std::make_pair(fd, what)] = std::make_pair(cont_ok, cont_err);
        bool mod = modes.count(fd);
        if (mod)
            what |= modes[fd];
        struct epoll_event ev;
        ev.events = what;
        ev.data.fd = fd;
        if (mod)
            epoll_ctl(epollfd_, EPOLL_CTL_MOD, fd, &ev);
        else
            epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &ev);
        modes[fd] = what;
        count++;
    }

    void unsubscribe(int fd, int what)
    {
        if (!queue.count(std::make_pair(fd, what)))
            throw std::runtime_error("Such element was not found in the queue");
        queue.erase(std::make_pair(fd, what));
        modes[fd] &= ~what;
        struct epoll_event ev;
        ev.events = modes[fd];
        ev.data.fd = fd;
        epoll_ctl(epollfd_, EPOLL_CTL_MOD, fd, &ev);
        count--;
    }

    void cycle()
    {
        struct epoll_event events[count];
        int nfds = 0;
        if (count > 0)
            nfds = epoll_wait(epollfd_, events, count, -1);
        if (nfds == -1)
            throw std::runtime_error("epoll_wait error");
        for (int i = 0; i < nfds; i++)
        {
            int fd = events[i].data.fd;
            if (events[i].events & EPOLLERR)
            {
                for (auto it : queue)
                    if (it.first.first == fd)
                    {
                        std::function<void()> cont = it.second.second;
                        unsubscribe(it.first.first, it.first.second);
                        cont();
                    }
            }
            else
            {
                int what = events[i].events;
                while (what != 0)
                {
                    int flag = what - (what & (what - 1));
                    if (modes[fd] & flag)
                    {
                        std::function<void()> cont = queue[std::make_pair(fd, flag)].first;
                        unsubscribe(fd, flag);
                        cont();
                    }
                    what ^= flag;
                }
            }
        }
    }

};

