#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char* read_buffer;
char* write_buffer;
int buf_size;
int readed;
int available;

int get_next_token()
{
    int long_string = 0;
    int ready = 0;
    while (ready == 0)
    {
        int ret = 1;
        ret = read(0, read_buffer + readed, buf_size - readed);
        if (ret <= 0)
        {
            if ((readed > 0) && (long_string == 0) && 
                    ((readed < buf_size) || (read_buffer[readed - 1] == '\n')))
            {
                memcpy(write_buffer, read_buffer, readed * sizeof(char));
                if (read_buffer[readed - 1] != '\n')
                {
                    read_buffer[readed] = '\n';
                    readed++;
                }
                available = readed;
            }
            return ret;
        }
        readed += ret;
        int start = 0;
        int found_nl = 0;
        if (long_string != 0)
        {
            while ((found_nl == 0) && (start < readed))
            {
                if (read_buffer[start] == '\n')
                {
                    found_nl = 1;
                }
                start++;
            }
            long_string = 0;
        }
        found_nl = 0;
        int end = start;
        while ((found_nl == 0) && (end < readed))
        {
            if (read_buffer[end] == '\n') 
            {
                found_nl = 1;
            }
            end++;
        }
        if ((found_nl != 0) || (start != 0))
        {
            if (found_nl != 0)
            {
                available = end - start;
                memcpy(write_buffer, read_buffer + start, 
                        available * sizeof(char));
                start = end;
                ready = 1;
            }
            readed -= start;
            memmove(read_buffer, read_buffer + start, readed * sizeof(char));
        }
        else
        {
            if (readed == buf_size)
            {
                long_string = 1;
                readed = 0;
            }
        }
    }
    return 1;
}

void print()
{
    int written = 0;
    int ret;
    while (written < available)
    {
        ret = write(1, write_buffer + written, available);
        written += ret;
        if (ret < 0)
        {
            return;
        }
    }
    written = 0;
    while (written < available)
    {
        ret = write(1, write_buffer + written, available);
        written += ret;
        if (ret < 0)
        {
            return;
        }
    }
    available = 0;
}

int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        buf_size = 0;
        int i;
        for (i = 0; argv[1][i] != 0; i++)
        {
            buf_size *= 10;
            buf_size += argv[1][i] - '0';
        }
        buf_size++;
    }
    else
    {
        buf_size = 5;
    }
    read_buffer = malloc(buf_size);
    write_buffer = malloc(buf_size);
    readed = 0;
    available = 0;
    while (get_next_token() > 0)
    {
        print();
    }
    print();
    free(read_buffer);
    free(write_buffer);
    return 0;
}
