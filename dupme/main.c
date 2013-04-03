#include <string.h>
#include <stdlib.h>
#include <unistd.h>


int get_next_token(char *read_buffer, char *write_buffer, const int buf_size, 
        int *len, int *available)
{
    int long_string = 0;
    int ready = 0;
    while (ready == 0)
    {
        int ret = read(0, read_buffer + *len, buf_size - *len);
        if (ret <= 0)
        {
            if ((*len > 0) && (long_string == 0) && 
                    ((*len < buf_size) || (read_buffer[(*len) - 1] == '\n')))
            {
                memcpy(write_buffer, read_buffer, (*len) * sizeof(char));
                if (write_buffer[(*len) - 1] != '\n')
                {
                    write_buffer[*len] = '\n';
                    (*len)++;
                }
                *available = *len;
            }
            return ret;
        }
        (*len) += ret;
        int start = 0;
        int found_nl = 0;
        if (long_string != 0)
        {
            while ((found_nl == 0) && (start < *len))
            {
                if (read_buffer[start] == '\n')
                {
                    found_nl = 1;
                }
                start++;
            }
            if (start < *len)
            {
                long_string = 0;
            }
        }
        found_nl = 0;
        int end = start;
        while ((found_nl == 0) && (end < *len))
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
                *available = end - start;
                memcpy(write_buffer, read_buffer + start, 
                        *available * sizeof(char));
                start = end;
                ready = 1;
            }
            (*len) -= start;
            memmove(read_buffer, read_buffer + start, (*len) * sizeof(char));
        }
        else
        {
            if (*len == buf_size)
            {
                long_string = 1;
                *len = 0;
            }
        }
    }
    return 1;
}

void print(char *write_buffer, int *available, int count)
{
    int ret;
    for (; count > 0; count--)
    {
        int written = 0;
        while (written < *available)
        {
            ret = write(1, write_buffer + written, *available);
            written += ret;
            if (ret < 0)
            {
                return;
            }
        }
    }
    *available = 0;
}

int main(int argc, char *argv[])
{
    int buf_size;
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
    char *read_buffer = malloc(buf_size);
    char *write_buffer = malloc(buf_size);
    int len = 0;
    int available = 0;
    while (get_next_token(read_buffer, write_buffer, buf_size, &len, &available) > 0)
    {
        print(write_buffer, &available, 2);
    }
    print(write_buffer, &available, 2);
    free(read_buffer);
    free(write_buffer);
    return 0;
}
