// NAME: Justin Hong
// EMAIL: justinjh@ucla.edu
// ID: 604565186

#define _GNU_SOURCE
#include <stdio.h>
#include <mcrypt.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>

const int BUFSIZE = 1024;
int encrypting = 0;
pid_t cpid;
int pipefd1[2];
int pipefd2[2];
int sockfd;
int keysize;
MCRYPT encrypt_td, decrypt_td;

void signal_handler(int sig_num)
{
    if (sig_num == SIGPIPE)
        exit(0);
}

void exiting()
{
	int status = 0;
    waitpid(cpid, &status, 0);
    unsigned int sig = ((unsigned int)status) & 0x7F;
    unsigned int stat = (((unsigned int)status) >> 8) & 0xFF;
    fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", sig, stat);
	close(0);
	close(1);
	close(pipefd1[1]);
	close(pipefd2[0]);
	close(sockfd);
}

char* getKey(char* file)
{
    struct stat st;
    int keyfd = open(file, O_RDONLY);
    if (fstat(keyfd, &st) < 0)
    {
        fprintf(stderr, "Invalid key");
        exit(1);
    }
    keysize = st.st_size;
    char* keybuf = (char*) malloc(sizeof(char)*keysize);
    if (read(keyfd, keybuf, keysize) < 0)
    {
        fprintf(stderr, "Error reading key: %s\n", strerror(errno));
        exit(1);
    }
    return keybuf;
}

void encrypt_decrypt_init(char* key, int keysize)
{
    encrypt_td = mcrypt_module_open("twofish", NULL, "cfb", NULL);
    if (encrypt_td == MCRYPT_FAILED)
    {
        fprintf(stderr, "Encryption module error: %s\n", strerror(errno));
        exit(1);
    }
    if (mcrypt_generic_init(encrypt_td, key, keysize, NULL))
    {
        fprintf(stderr, "Encryption initialization error: %s\n", strerror(errno));
        exit(1);
    }
    decrypt_td = mcrypt_module_open("twofish", NULL, "cfb", NULL);
    if (decrypt_td == MCRYPT_FAILED)
    {
        fprintf(stderr, "Decryption module error: %s\n", strerror(errno));
        exit(1);
    }
    if (mcrypt_generic_init(decrypt_td, key, keysize, NULL))
    {
        fprintf(stderr, "Decryption initialization error: %s\n", strerror(errno));
        exit(1);
    }
}

int main(int argc, char **argv)
{
	int c;
	int portno;
	struct sockaddr_in serv_addr, cli_addr;
	int newsockfd;
	int clilen;
	while (1)
	{
		static struct option long_options[] =
		{
			{"port", required_argument, 0, 'p'},
			{"encrypt", required_argument, 0, 'e'},
			{0, 0, 0, 0}
		};
		c = getopt_long(argc, argv, "p:e:", long_options, NULL);
		if (c == -1)
			break;
		switch(c)
		{
			case 'p':
				portno = atoi(optarg);
				break;
			case 'e':
				encrypting = 1;
				char* key = getKey(optarg);
				encrypt_decrypt_init(key, keysize);
				break;
			default:
				fprintf(stderr, "usage: %s --port portnumber [--encrypt filename]\n", argv[0]);
				exit(1);
		}
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        fprintf(stderr, "Error opening socket: %s\n", strerror(errno));
        exit(1);
    }
    bzero((char*) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
    {
    	fprintf(stderr, "Error binding: %s\n", strerror(errno));
    	exit(1);
    }
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr*) &cli_addr, &clilen);
    if (newsockfd < 0)
    {
    	fprintf(stderr, "Error accepting: %s\n", strerror(errno));
    	exit(1);
    }
    if (pipe(pipefd1) == -1 || pipe(pipefd2) == -1)
    {
        fprintf(stderr, "Error creating pipes: %s\n", strerror(errno));
        exit(1);
    }
    signal(SIGPIPE, signal_handler);
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
    atexit(exiting);
    struct pollfd fds[2];
    fds[0].fd = newsockfd;
    fds[1].fd = pipefd2[0];
    fds[2].fd = sockfd;
    fds[0].events = POLLIN | POLLHUP | POLLERR | POLLRDHUP;
    fds[1].events = POLLIN | POLLHUP | POLLERR;
    fds[2].events = POLLHUP | POLLERR | POLLRDHUP;
    char* buf = (char*) malloc(sizeof(char)*BUFSIZE);
    int offset = 0;
    int socketRead = 0;
    int shellEOF = 0;
    int newline = 0;
    ssize_t bytes;
    while (1)
    {
        int ready = poll(fds, 3, 0);
        if (ready > 0)
        {
            newline = 0;
            if (fds[0].revents & POLLIN)
            {
                bytes = read(newsockfd, buf+offset, 1);
                socketRead = 1;
            }
            if (fds[1].revents & POLLIN)
            {
                bytes = read(pipefd2[0], buf+offset, 1);
                socketRead = 0;
            }
            if (fds[1].revents & POLLHUP || fds[1].revents & POLLERR)
            {
                exit(0);
            }
            if (fds[0].revents & POLLHUP || fds[0].revents & POLLERR || fds[0].revents & POLLRDHUP)
            {
            	kill(cpid, SIGTERM);
            }
            if (fds[2].revents & POLLHUP || fds[2].revents & POLLERR || fds[2].revents & POLLRDHUP)
            {
                kill(cpid, SIGTERM);
            }
        }
        else if (ready == -1)
        {
            fprintf(stderr, "Polling error: %s\n", strerror(errno));
            exit(1);
        }
        else
            continue;
        if (bytes == -1)
        {
            fprintf(stderr, "Error reading: %s\n", strerror(errno));
            exit(1);
        }
        if (bytes == 0)
            exit(0);
        if (!socketRead && *(buf+offset) == 0x0A)
            newline = 1;
        char temp_buf[2] = {0x0D, 0x0A};
        if (encrypting && socketRead)
        {
        	if (mdecrypt_generic(decrypt_td, buf+offset, 1) != 0)
        	{
        		fprintf(stderr, "Decryption error: %s\n", strerror(errno));
        		exit(1);
        	}
        }
        if (encrypting && !socketRead)
        {
            if (newline)
            {
                if (mcrypt_generic(encrypt_td, temp_buf, 2) != 0)
                {
                    fprintf(stderr, "Encryption error: %s\n", strerror(errno));
                    exit(1);
                }
            }
            else
            {
                if (mcrypt_generic(encrypt_td, buf+offset, 1) != 0)
                {
                    fprintf(stderr, "Encryption error: %s\n", strerror(errno));
                    exit(1);
                }
            }
        }
        if (socketRead)
        {
	        int curr = *(buf + offset);
	        if (curr == 0x04)
	        {
	            close(pipefd1[1]);
	            shellEOF = 1;
	        }
	        if (curr == 0x03)
	            kill(cpid, SIGINT);
    	}
        if (!socketRead)
        {
            if (newline)
            {
                if (write(newsockfd, temp_buf, 1) == -1)
                {
                    fprintf(stderr, "Error writing: %s\n", strerror(errno));
                    exit(1);
                }
                if (write(newsockfd, temp_buf+1, 1) == -1)
                {
                    fprintf(stderr, "Error writing: %s\n", strerror(errno));
                    exit(1);
                }
            }
            else
            {
                if (write(newsockfd, buf+offset, 1) == -1)
                {
                    fprintf(stderr, "Error writing: %s\n", strerror(errno));
                    exit(1);
                }
            }
    	}
        if (socketRead && !shellEOF)
        {
            if (write(pipefd1[1], buf+offset, 1) == -1)
            {
                fprintf(stderr, "Error writing: %s\n", strerror(errno));
                exit(1);
            }
        }
        offset++;
    }
    exit(0);
}
