#include "epollfd.h"
#include <iostream>
#include <string>

bool quit = false;

void subscribe(epollfd &e)
{
    e.subscribe(0, EPOLLIN,
        [&e]()
        {
            std::cout << "OK" << std::endl;
            subscribe(e);
        }, 
        []()
        {
            std::cout << "Ne OK" << std::endl;
            quit = true;
        });
}

int main()
{
    epollfd e;
    subscribe(e);
    while (!quit)
    {
        e.cycle();
        std::string s;
        std::cin >> s;
    }
}
