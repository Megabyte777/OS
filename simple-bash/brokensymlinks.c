#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void find_broken_symlinks(char* const prefix)
{
    size_t len = strlen(prefix);
    char filename[1024];
    memcpy(filename, prefix, sizeof(char) * (len + 1));
    if (filename[len - 1] != '/')
    {
        filename[len] = '/';
        len++;
        filename[len] = 0;
    }
    DIR *dirp = opendir(prefix);
    if (dirp == NULL)
    {
        return;
    }
    struct dirent *file;
    while ((file = readdir(dirp)) != NULL)
    {
        if (strcmp(file->d_name, ".") && strcmp(file->d_name, ".."))
        {
            strcat(filename, file->d_name);
            if ((file->d_type == DT_LNK) && (access(filename, F_OK) == -1))
            {
                printf("%s\n", filename);
            }
            else if(file->d_type == DT_DIR)
            {
                find_broken_symlinks(filename);
            }
            filename[len] = 0;
        }
    }
    closedir(dirp);
}

int main(int argc, char *argv[])
{
    if (getopt(argc, argv, "") != -1)
    {
        fprintf(stderr, "Unexpected options\n");
        exit(EXIT_FAILURE);
    }
    int i;
    for (i = 1; i < argc; i++)
    {
        find_broken_symlinks(argv[i]);
    }
    exit(EXIT_SUCCESS);
}
