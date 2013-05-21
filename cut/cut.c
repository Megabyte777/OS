#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void write_(int fd, char * const s, size_t len)
{
    size_t printed = 0;
    while (printed < len)
    {
        ssize_t res = write(fd, s + printed, len - printed);
        if (res < 0)
        {
            perror("Error while writing");
            exit(EXIT_FAILURE);
        }
        printed += res;
    }
}

void print_(char *buf, size_t pos, int field, char * const delim, size_t delim_len)
{
    size_t i;
    size_t begin = 0;
    int k = 1;
    for (i = 0; i != pos; i++)
    {
        if (delim != 0 && memchr(delim, buf[i], delim_len) != NULL)
        {
            if ((k == field) || (field == 0) || 
                    ((field < 0) && (k >= -field)))
            {
                write_(1, buf + begin, i - begin);
                write_(1, "\n", 1);
            }
            k++;
            begin = i + 1;
        }
    }
    if (begin < pos && (k == field || field == 0 || (field < 0 && k >= -field)))
    {
        write_(1, buf + begin, pos - begin);
        write_(1, "\n", 1);
    }
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
    char *IFS = getenv("IFS");
    if (IFS == NULL)
        IFS = " \n";
    size_t delim_lines_len = strlen(IFS);
    char *delim_lines = my_alloc(delim_lines_len + 1);
    memcpy(delim_lines, IFS, delim_lines_len + 1);
    size_t bufsize = 4096;
    int was_d = 0;
    int was_b = 0;
    int was_f = 0;
    int opt;
    int f = 0;
    char *delim;
    size_t delim_len;
    while ((opt = getopt(argc, argv, "d:b:f:")) != -1)
    {
        switch (opt)
        {
        case 'd':
            if (!was_d)
            {
                delim_len = strlen(optarg);
                delim = my_alloc(delim_len + 1);
                memcpy(delim, optarg, delim_len + 1);
                was_d = 1;
            }
            else
                exit(EXIT_FAILURE);
            break;
        case 'b':
            if (!was_b)
            {
                bufsize = atoi(optarg);
                was_b = 1;
            }
            else
                exit(EXIT_FAILURE);
            break;
        case 'f':
            if (!was_f)
            {
                f = atoi(optarg);
                if (optarg[strlen(optarg) - 1] == '-')
                    f = -f;
                was_f = 1;
            }
            else
                exit(EXIT_FAILURE);
            break;
        default:
            exit(EXIT_FAILURE);
        }
    }
    char *buf = my_alloc(bufsize);
    int finish = 0;
    size_t len = 0;
    while (!finish)
    {
        if (len >= bufsize)
            exit(EXIT_FAILURE);
        ssize_t res = read(0, buf + len, bufsize - len);
        if (res < 0)
            exit(EXIT_FAILURE);
        if (res == 0)
            finish = 1;
        size_t delpos = len;
        len += res;
        if (res == 0)
        {
            if (len != 0)
                buf[len] = IFS[0];
            len++;
            finish = 1;
        }
        while (delpos != len)
        {
            delpos = 0;
            while (delpos < len &&
                    memchr(delim_lines, buf[delpos], delim_lines_len) == NULL)
                delpos++;
            if (delpos < len &&
                    memchr(delim_lines, buf[delpos], delim_lines_len) != NULL)
            {
                print_(buf, delpos, f, delim, delim_len);
                len -= delpos + 1;
                memmove(buf, buf + delpos + 1, len);
            }
        }
    }
    free(buf);
    free(delim);
    free(delim_lines);
    return 0;
}
