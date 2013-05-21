#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

void write_(int fd, char * const s, size_t len)
{
    size_t printed = 0;
    while (printed < len)
    {
        ssize_t res = write(fd, s + printed, len - printed);
        if (res < 0)
            _exit(EXIT_FAILURE);
        printed += res;
    }
}

void run_cmd(char * const cmd, char * const args[], size_t arg_pos)
{
    if (fork())
    {
        int status;
        wait(&status);
        if (WIFEXITED(status) && (WEXITSTATUS(status) == 0))
        {
            write_(1, args[arg_pos], strlen(args[arg_pos]));
            write_(1, "\n", 1);
        }
    }
    else
    {
        int fd = open("/dev/null", O_RDWR);
        dup2(fd, 0);
        dup2(fd, 1);
        dup2(fd, 2);
        close(fd);
        execvp(cmd, args);
    }
}

void* my_alloc(size_t size)
{
    void *data = malloc(size);
    if (data == 0)
        _exit(EXIT_FAILURE);
    return data;
}

int main(int argc, char * argv[])
{
    //READING ARGUMENTS
    char sep = ' ';
    int opt;
    size_t buf_size = 4096;
    while ((opt = getopt(argc, argv, "nzb:")) != -1)
    {
        switch (opt)
        {
        case 'n':
            if (sep != ' ' && sep != '\n')
                exit(EXIT_FAILURE);
            sep = '\n';
            break;
        case 'z':
            if (sep != ' ' && sep != 0)
                exit(EXIT_FAILURE);
            sep = 0;
            break;
        case 'b':
            buf_size = atoi(optarg);
            break;
        default:
            exit(EXIT_FAILURE);
        }
    }
    if (sep == ' ')
        sep = '\n';
    if (optind >= argc)
        exit(EXIT_FAILURE);
    char * cmd = argv[optind];
    size_t arg_pos = argc - optind;
    char * args[argc - optind + 2];
    args[arg_pos] = my_alloc(buf_size);
    memcpy(args, argv + optind, sizeof(char*) * (argc - optind));
    args[arg_pos + 1] = 0;
    //END OF READING ARGUMENTS
    char * buf = my_alloc(buf_size + 1);
    args[argc] = my_alloc(buf_size);
    size_t len = 0;
    int end = 0;
    while (!end)
    {
        if (len >= buf_size)
            exit(EXIT_FAILURE);
        ssize_t res = read(0, buf + len, buf_size - len);
        if (res < 0)
            exit(EXIT_FAILURE);
        size_t delpos = len;
        len += res;
        if (res == 0)
        {
            if (len != 0)
                buf[len] = sep;
            len++;
            end = 1;
        }
        while (delpos != len)
        {
            for (delpos = 0; (delpos < len) && (buf[delpos] != sep); delpos++)
            {}
            if (delpos != len && buf[delpos] == sep)
            {
                memcpy(args[arg_pos], buf, delpos);
                args[arg_pos][delpos] = 0;
                run_cmd(cmd, args, arg_pos);
                memmove(buf, buf + delpos + 1, len - delpos - 1);
                len -= delpos + 1;
            }
        }
    }
    free(buf);
    free(args);
    free(args[argc]);
    exit(EXIT_SUCCESS);
}
