//  NAME: Justin Hong
//  EMAIL: justinjh@ucla.edu
//  ID: 604565186

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

void handler(int signum)
{
    fprintf(stderr, "Segmentation fault caught");
    exit(4);
}

void sfault()
{
    char* seg = NULL;
    char n = *seg;
}

int main(int argc, char **argv)
{
    int c;
    char* input = NULL;
    char* output = NULL;
    int segfault = 0;
    int fd0 = 0;
    int fd1 = 1;
    while (1)
    {
        static struct option long_options[] =
        {
            {"input", required_argument, 0, 'i'},
            {"output", required_argument, 0, 'o'},
            {"segfault", no_argument, 0, 's'},
            {"catch", no_argument, 0, 'c'},
            {0, 0, 0, 0}
        };
        c = getopt_long(argc, argv, "i:o:sc", long_options, NULL);
        if (c == -1)
            break;
        switch(c)
        {
            case 'i':
                input = optarg;
                break;
            case 'o':
                output = optarg;
                break;
            case 's':
                segfault = 1;
                break;
            case 'c':
                signal(SIGSEGV, handler);
                break;
            default:
                fprintf(stderr, "usage: lab0 [--input filename] [--output filename] [--segfault] [--catch]");
                exit(1);
        }
    }
    if (segfault)
        sfault();
    if (input)
    {
        fd0 = open(input, O_RDONLY);
        if (fd0 >= 0)
        {
            close(0);
            dup(fd0);
            close(fd0);
        }
        else
        {
            fprintf(stderr, "Error opening file: %s\n", strerror(errno));
            exit(2);
        }
    }
    if (output)
    {
        fd1 = creat(output, 0666);
        if (fd1 >= 0)
        {
            close(1);
            dup(fd1);
            close(fd1);
        }
        else
        {
            fprintf(stderr, "Error creating file: %s\n", strerror(errno));
            exit(3);
        }
    }
    char* buf;
    buf = (char*) malloc(sizeof(char));
    while (read(0, buf, 1) > 0)
        write(1, buf, 1);
    free(buf);
    exit(0);
}
