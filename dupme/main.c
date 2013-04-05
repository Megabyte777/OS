#include <string.h>
#include <stdlib.h>
#include <unistd.h>


int get_next_token(char *read_buffer, char *write_buffer, const int buf_size, 
        int *len, int *available)
{
    int long_string = 0;
    int first_overflow = 0;
    int ready = 0;
    for (;;)
    {
        int ret = read(0, read_buffer + *len, buf_size - *len);
        if (ret <= 0)
        {
            memcpy(write_buffer, read_buffer, (*len) * sizeof(char));
            if (first_overflow != 0)
            {
                *available = buf_size + 1;
            }
            else if (long_string == 0)
            {
                if (((*len) != 0) && (write_buffer[(*len) - 1] != '\n'))
                {
                    write_buffer[*len] = '\n';
                    (*len)++;
                }
                *available = *len;
            }
            return ret;
        }
        (*len) += ret;
        if (first_overflow == 1)
        {
            if (read_buffer[0] == '\n')
            {
                *available = buf_size + 1;
                memmove(read_buffer, read_buffer + 1, ((*len) - 1) * sizeof(char));
                (*len)--;
                return 1;
            }
            else
            {
                long_string = 1;
                first_overflow = 0;
            }
        }
        int end = 0;
        if (long_string != 0)
        {
            while ((end < (*len - 1)) && (read_buffer[end] != '\n'))
            {
                end++;
            }
            if (read_buffer[end] == '\n')
            {
                long_string = 0;
            }
            end++;
            (*len) -= end;
            memmove(read_buffer, read_buffer + end, (*len) * sizeof(char));
        }
        end = 0;
        while ((end < *len) && (read_buffer[end] != '\n'))
        {
            end++;
        }
        if (end >= *len)
        {
            if (*len == buf_size)
            {
                first_overflow = 1;
                memcpy(write_buffer, read_buffer, (*len) * sizeof(char));
                write_buffer[buf_size] = '\n';
                *len = 0;
            }
        }
        else
        {
            long_string = 0;
            *available = end + 1;
            memcpy(write_buffer, read_buffer, (*available) * sizeof(char));
            (*len) -= end + 1;
            memmove(read_buffer, read_buffer + end + 1, (*len) * sizeof(char));
            return 1;
        }
    }
}

void print(char *write_buffer, int *available, int count)
{
    int ret;
    for (; count > 0; count--)
    {
        int written = 0;
        while (written < *available)
        {
            ret = write(1, write_buffer + written, *available - written);
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
    }
    else
    {
        buf_size = 5;
    }
    char *read_buffer = malloc(buf_size);
    char *write_buffer = malloc(buf_size + 1);
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
