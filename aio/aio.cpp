#include "epollfd.h"
#include "aio.h"
#include "buffer.h"
#include <cstdio>

char buf[256];

int main()
{
    epollfd e;
    reader read_buf(0, buf, 256);
    aio_manager aiom(&e);
    aiom.read(0, &read_buf,
            []()
            {
                printf("%s", buf);
            },
            []()
            {
                printf("Error\n");
            });
    aiom.cycle();
}
