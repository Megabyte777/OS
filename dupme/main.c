#include <string.h>
#include <stdlib.h>
#include <unistd.h>

typedef enum state
{
    NORMAL,
    LONG_STRING,
    IGNORING
} state;

int get_next_token(char *read_buffer, char *write_buffer, const int buf_size,
        int *len, int *available)
{
    state status = NORMAL;
    for (;;)
    {
        int ret = read(0, read_buffer + *len, buf_size - *len);
        if (ret <= 0)
        {
            memcpy(write_buffer, read_buffer, *len);
            switch (status)
            {
            case NORMAL:
                if (*len != 0 && write_buffer[*len - 1] != '\n')
                {
                    write_buffer[*len] = '\n';
                    (*len)++;
                }
                *available = *len;
                break;
            case LONG_STRING:
                *available = buf_size + 1;
            default:
                break;
            }
            return ret;
        }
        (*len) += ret;
        if (status == LONG_STRING)
        {
            if (read_buffer[0] == '\n')
            {
                *available = buf_size + 1;
                memmove(read_buffer, read_buffer + 1, *len - 1);
                (*len)--;
                return 1;
            }
            else
                status = IGNORING;
        }
        int end = 0;
        if (status == IGNORING)
        {
            while (end < *len - 1 && read_buffer[end] != '\n')
                end++;
            if (read_buffer[end] == '\n')
                status = NORMAL;
            end++;
            (*len) -= end;
            memmove(read_buffer, read_buffer + end, *len);
        }
        end = 0;
        while (end < *len && read_buffer[end] != '\n')
            end++;
        if (end == *len)
        {
            if (*len == buf_size)
            {
                status = LONG_STRING;
                memcpy(write_buffer, read_buffer, *len);
                write_buffer[buf_size] = '\n';
                *len = 0;
            }
        }
        else
        {
            status = NORMAL;
            *available = end + 1;
            memcpy(write_buffer, read_buffer, *available);
            (*len) -= end + 1;
            memmove(read_buffer, read_buffer + end + 1, *len);
            return 1;
        }
    }
}

void print(char *write_buffer, int *available, int count)
{
    for (; count > 0; count--)
    {
        int written = 0;
        while (written < *available)
        {
            int ret = write(1, write_buffer + written, *available - written);
            written += ret;
            if (ret < 0)
                _exit(EXIT_FAILURE);
        }
    }
    *available = 0;
}

void* my_alloc(size_t size)
{
    void *data = malloc(size);
    if (data == 0)
        _exit(EXIT_FAILURE);
    return data;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
        return 1;
    int buf_size = atoi(argv[1]);
    char *read_buffer = my_alloc(buf_size);
    char *write_buffer = my_alloc(buf_size + 1);
    int len = 0;
    int available = 0;
    while (get_next_token(read_buffer, write_buffer, buf_size, &len, &available) > 0)
        print(write_buffer, &available, 2);
    print(write_buffer, &available, 2);
    free(read_buffer);
    free(write_buffer);
    return 0;
}
