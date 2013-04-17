#include <unistd.h>
#include <string.h>
#include <stdlib.h>

char *buf;
size_t bufsize;
char *delim_lines;
size_t delim_lines_len;
char *delim;
size_t delim_len;

void _write(int fd, char * const s, size_t len)
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

void _print(char *buf, size_t pos, int field)
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
                _write(1, buf + begin, (i - begin) * sizeof(char));
                _write(1, "\n", 1);
            }
            k++;
            begin = i + 1;
        }
    }
    if ((begin < pos) && 
            ((k == field) || (field == 0) || ((field < 0) && (k >= -field))))
    {
        _write(1, buf + begin, (pos - begin) * sizeof(char));
        _write(1, "\n", 1);
    }
}

int main(int argc, char *argv[])
{
    char *IFS = getenv("IFS");
    if (IFS == NULL)
    {
        IFS = " \n";
    }
    delim_lines_len = strlen(IFS);
    delim_lines = malloc((delim_lines_len + 1) * sizeof(char));
    memcpy(delim_lines, IFS, (delim_lines_len + 1) * sizeof(char));
    bufsize = 4096;
    int was_d = 0;
    int was_b = 0;
    int was_f = 0;
    int opt;
    int f = 0;
    while ((opt = getopt(argc, argv, "d:b:f:")) != -1)
    {
        switch (opt)
        {
        case 'd':
            if (was_d == 0)
            {
                delim_len = strlen(optarg);
                delim = malloc((delim_len + 1) * sizeof(char));
                memcpy(delim, optarg, (delim_len + 1) * sizeof(char));
                was_d = 1;
            }
            else
            {
                exit(EXIT_FAILURE);
            }
            break;
        case 'b':
            if (was_b == 0)
            {
                bufsize = atoi(optarg);
                was_b = 1;
            }
            else
            {
                exit(EXIT_FAILURE);
            }
            break;
        case 'f':
            if (was_f == 0)
            {
                f = atoi(optarg);
                if (optarg[strlen(optarg) - 1] == '-')
                {
                    f = -f;
                }
                was_f = 1;
            }
            else
            {
                exit(EXIT_FAILURE);
            }
            break;
        default:
            exit(EXIT_FAILURE);
        }
    }
    buf = malloc(bufsize);
    int finish = 0;
    size_t len = 0;
    while (finish == 0)
    {
        if (len >= bufsize)
        {
            exit(EXIT_FAILURE);
        }
        ssize_t res = read(0, buf + len, bufsize - len);
        if (res < 0)
        {
            exit(EXIT_FAILURE);
        }
        if (res == 0)
        {
            finish = 1;
        }
        size_t delpos = len;
        len += res;
        if (res == 0)
        {
            if (len != 0)
            {
                buf[len] = IFS[0];
            }
            len++;
            finish = 1;
        }
        while (delpos != len)
        {
            delpos = 0;
            while ((delpos < len) &&
                    (memchr(delim_lines, buf[delpos], delim_lines_len) == NULL))
            {
                delpos++;
            }
            if ((delpos < len) &&
                    (memchr(delim_lines, buf[delpos], delim_lines_len) != NULL))
            {
                _print(buf, delpos, f);
                len -= delpos + 1;
                memmove(buf, buf + delpos + 1, 
                        sizeof(char) * len);
            }
        }
    }
    free(buf);
    free(delim);
    free(delim_lines);
}
