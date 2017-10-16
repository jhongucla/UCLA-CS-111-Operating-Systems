// NAME: Justin Hong
// EMAIL: justinjh@ucla.edu
// ID: 604565186

#define _GNU_SOURCE
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <poll.h>

const int BUFSIZE = 1024;

struct termios saved_state;
int shell = 0;
pid_t cpid;

void exiting()
{
    tcsetattr(0, TCSANOW, &saved_state);
    if (shell)
    {
        int status = 0;
        waitpid(cpid, &status, 0);
        unsigned int sig = ((unsigned int)status) & 0x7F;
        unsigned int stat = (((unsigned int)status) >> 8) & 0xFF;
        fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", sig, stat);
    }
}

void signal_handler(int sig_num)
{
    if (sig_num == SIGPIPE)
        exit(0);
}

int main(int argc, char **argv)
{
    int c;
    while (1)
    {
        static struct option long_options[] =
        {
            {"shell", no_argument, 0, 's'},
            {0, 0, 0, 0}
        };
        c = getopt_long(argc, argv, "s", long_options, NULL);
        if (c == -1)
            break;
        switch(c)
        {
            case 's':
                signal(SIGPIPE, signal_handler);
                shell = 1;
                break;
            default:
                fprintf(stderr, "usage: lab1a [--shell]\n");
                exit(1);
        }
    }
    
    struct termios change_state;
    if (tcgetattr(0, &saved_state) == -1)
    {
        fprintf(stderr, "Error getting terminal modes: %s\n", strerror(errno));
        exit(1);
    }
    change_state = saved_state;
    change_state.c_iflag = ISTRIP;
    change_state.c_oflag = 0;
    change_state.c_lflag = 0;
    
    if (tcsetattr(0, TCSANOW, &change_state) == -1)
    {
        fprintf(stderr, "Error changing terminal modes: %s\n", strerror(errno));
        exit(1);
    }
    
    atexit(exiting);
    int pipefd1[2];
    int pipefd2[2];
    if (shell)
    {
        if (pipe(pipefd1) == -1 || pipe(pipefd2) == -1)
        {
            fprintf(stderr, "Error creating pipes: %s\n", strerror(errno));
            exit(1);
        }
        cpid = fork();
        if (cpid == -1)
        {
            fprintf(stderr, "Error creating child process: %s\n", strerror(errno));
            exit(1);
        }
        if (cpid == 0)
        {
            close(pipefd1[1]);
            close(pipefd2[0]);
            dup2(pipefd1[0], 0);
            dup2(pipefd2[1], 1);
            dup2(pipefd2[1], 2);
            close(pipefd1[0]);
            close(pipefd2[1]);
            if (execvp("/bin/bash", NULL) == -1)
            {
                fprintf(stderr, "Exec error: %s\n", strerror(errno));
                exit(1);
            }
        }
        else
        {
            close(pipefd1[0]);
            close(pipefd2[1]);
        }
    }
    
    struct pollfd fds[2];
    if (shell)
    {
        fds[0].fd = 0;
        fds[1].fd = pipefd2[0];
        fds[0].events = POLLIN | POLLHUP | POLLERR;
        fds[1].events = POLLIN | POLLHUP | POLLERR;
    }
    char* buf;
    int offset = 0;
    int keyboardRead = 0;
    int shellEOF = 0;
    ssize_t bytes;
    buf = (char*) malloc(sizeof(char)*BUFSIZE);
    while (1)
    {
        if (shell == 1)
        {
            int ready = poll(fds, 2, 0);
            if (ready > 0)
            {
                if (fds[0].revents & POLLIN)
                {
                    bytes = read(0, buf+offset, 1);
                    keyboardRead = 1;
                }
                if (fds[1].revents & POLLIN)
                {
                    bytes = read(pipefd2[0], buf+offset, 1);
                    keyboardRead = 0;
                }
                if (fds[0].revents & POLLHUP || fds[0].revents & POLLERR || fds[1].revents & POLLHUP || fds[1].revents & POLLERR)
                {
                    shellEOF = 1;
                }
                
            }
            else if (ready == -1)
            {
                fprintf(stderr, "Polling error: %s\n", strerror(errno));
                exit(1);
            }
            else
                continue;
        }
        else
            bytes = read(0, buf+offset, 1);
        if (bytes == -1)
        {
            fprintf(stderr, "Error reading: %s\n", strerror(errno));
            exit(1);
        }
        if (bytes == 0)
            exit(0);
        int curr = *(buf + offset);
        if (curr == 0x04)
        {
            if (shell == 1)
            {
                close(pipefd1[0]);
                close(pipefd1[1]);
                close(pipefd2[0]);
                close(pipefd2[1]);
                shellEOF = 1;
            }
            else
                exit(0);
        }
        if (curr == 0x03 && shell && keyboardRead)
            kill(cpid, SIGINT);
        if (curr == 0x0A || curr == 0x0D)
        {
            char temp_buf[2] = {0x0D, 0x0A};
            if (write(1, temp_buf, 2) == -1)
            {
                fprintf(stderr, "Error writing: %s\n", strerror(errno));
                exit(1);
            }
            if (keyboardRead && shell)
                if (write(pipefd1[1], temp_buf+1, 1) == -1)
                {
                    fprintf(stderr, "Error writing: %s\n", strerror(errno));
                    exit(1);
                }
            offset++;
            continue;
        }
        write(1, buf+offset, 1);
        if (shell && keyboardRead && !shellEOF)
            if (write(pipefd1[1], buf+offset, 1) == -1)
            {
                fprintf(stderr, "Error writing: %s\n", strerror(errno));
                exit(1);
            }
        offset++;
        if (shellEOF)
            exit(0);
    }
    exit(0);
}
