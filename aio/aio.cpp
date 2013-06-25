#include "epollfd.h"
#include "aio.h"
#include "buffer.h"
#include <cstdio>

char buf[256];

int main()
{
    epollfd e;
    reader read_buf(0, buf, 256);
    ARead aread(&e, 0, &read_buf,
            []()
            {
                printf("%s\n", buf);
            },
            []()
            {
                printf("Error\n");
            });
    e.cycle();
}
