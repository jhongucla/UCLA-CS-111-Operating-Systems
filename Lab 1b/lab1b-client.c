// NAME: Justin Hong
// EMAIL: justinjh@ucla.edu
// ID: 604565186

#define _GNU_SOURCE
#include <stdio.h>
#include <mcrypt.h>
#include <termios.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <poll.h>

const int BUFSIZE = 1024;
struct termios saved_state;
int logging = 0;
int encrypting = 0;
int keysize;
MCRYPT encrypt_td, decrypt_td;

void exiting()
{
    if (encrypting)
    {
        mcrypt_generic_deinit(encrypt_td);
        mcrypt_module_close(encrypt_td);
        mcrypt_generic_deinit(decrypt_td);
        mcrypt_module_close(decrypt_td);
    }
    tcsetattr(0, TCSANOW, &saved_state);
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
    int sockfd;
    int log_fd;
    char* logfile = NULL;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    while (1)
    {
        static struct option long_options[] =
        {
            {"port", required_argument, 0, 'p'},
            {"log", required_argument, 0, 'l'},
            {"encrypt", required_argument, 0, 'e'},
            {0, 0, 0, 0}
        };
        c = getopt_long(argc, argv, "p:l:e:", long_options, NULL);
        if (c == -1)
            break;
        switch(c)
        {
            case 'p':
                portno = atoi(optarg);
                break;
            case 'l':
                logging = 1;
                logfile = optarg;
                log_fd = creat(logfile, 0666);
                if (log_fd == -1)
                {
                    fprintf(stderr, "Error creating log file: %s\n", strerror(errno));
                    exit(1);
                }
                break;
            case 'e':
                encrypting = 1;
                char* key = getKey(optarg);
                encrypt_decrypt_init(key, keysize);
                break;
            default:
                fprintf(stderr, "usage: %s --port portnumber [--log filename] [--encrypt filename]\n", argv[0]);
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
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        fprintf(stderr, "Error opening socket: %s\n", strerror(errno));
        exit(1);
    }
    server = gethostbyname("localhost");
    if (server == NULL)
    {
        fprintf(stderr, "Error finding host: %s\n", strerror(errno));
        exit(1);
    }
    bzero((char*) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char*) server->h_addr, (char*) &serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
    {
        fprintf(stderr, "Error connecting: %s\n", strerror(errno));
        exit(1);
    }

    struct pollfd fds[2];
    fds[0].fd = 0;
    fds[1].fd = sockfd;
    fds[0].events = POLLIN | POLLHUP | POLLERR;
    fds[1].events = POLLIN | POLLHUP | POLLERR;
    char* buf = (char*) malloc(sizeof(char)*BUFSIZE);
    char* unencryptedbuf = (char*) malloc(sizeof(char));
    char* encryptedbuf = (char*) malloc(sizeof(char));
    int offset = 0;
    int keyboardRead = 0;
    int encrypted = 0;
    int newline = 0;
    ssize_t bytes;
    ssize_t log_bytes = 0;
    while (1)
    {
        int ready = poll(fds, 2, 0);
        if (ready > 0)
        {
            newline = 0;
            encrypted = 0;
            if (fds[0].revents & POLLIN)
            {
                bytes = read(0, buf+offset, 1);
                if (bytes == -1)
                {
                    fprintf(stderr, "Error reading: %s\n", strerror(errno));
                    exit(1);
                }
                if (bytes == 0)
                    exit(0);
                *unencryptedbuf = *(buf+offset);
                if (encrypting && *(buf+offset) != 0x0A && *(buf+offset) != 0x0D)
                {
                    if (mcrypt_generic(encrypt_td, buf+offset, 1) != 0)
                    {
                        fprintf(stderr, "Encryption error: %s\n", strerror(errno));
                        exit(1);
                    }
                    encrypted = 1;
                }
                keyboardRead = 1;
            }
            if (fds[1].revents & POLLIN)
            {
                bytes = read(sockfd, buf+offset, 1);
                if (bytes == -1)
                {
                    fprintf(stderr, "Error reading: %s\n", strerror(errno));
                    exit(1);
                }
                if (bytes == 0)
                    exit(0);
                *encryptedbuf = *(buf+offset);
                if (encrypting)
                {
                    if (mdecrypt_generic(decrypt_td, buf+offset, 1) != 0)
                    {
                        fprintf(stderr, "Decryption error: %s\n", strerror(errno));
                        exit(1);
                    }
                }
                keyboardRead = 0;
            }
            if (fds[0].revents & POLLHUP || fds[0].revents & POLLERR || fds[1].revents & POLLHUP || fds[1].revents & POLLERR)
            {
                exit(0);
            }
        }
        else if (ready == -1)
        {
            fprintf(stderr, "Polling error: %s\n", strerror(errno));
            exit(1);
        }
        else
            continue;
        int curr = *(buf + offset);
        if ((curr == 0x0A || curr == 0x0D) && !encrypted && keyboardRead)
        {
            char temp_buf[2] = {0x0D, 0x0A};
            if (write(1, temp_buf, 2) == -1)
            {
                fprintf(stderr, "Error writing: %s\n", strerror(errno));
                exit(1);
            }
            if (encrypting)
            {
                if (mcrypt_generic(encrypt_td, temp_buf+1, 1) != 0)
                {
                    fprintf(stderr, "Encryption error: %s\n", strerror(errno));
                    exit(1);
                }
            }
            if (write(sockfd, temp_buf+1, 1) == -1)
            {
                fprintf(stderr, "Error writing: %s\n", strerror(errno));
                exit(1);
            }
            if (logging)
            {
                char sent_string[14] = "SENT   bytes: ";
                sent_string[5] = '0' + bytes;
                write(log_fd, sent_string, 14);
                write(log_fd, temp_buf+1, 1);
                write(log_fd, "\n", 1);
            }
            newline = 1;
        }
        if (!keyboardRead)
        {
            if (write(1, buf+offset, 1) == -1)
            {
                fprintf(stderr, "Error writing: %s\n", strerror(errno));
                exit(1);
            }
            if (logging && encrypting)
            {
                char receive_string[18] = "RECEIVED   bytes: ";
                receive_string[9] = '0' + bytes;
                write(log_fd, receive_string, 18);
                write(log_fd, encryptedbuf, 1);
                write(log_fd, "\n", 1);
            }
            else if (logging && !encrypting)
            {
                char receive_string[18] = "RECEIVED   bytes: ";
                receive_string[9] = '0' + bytes;
                write(log_fd, receive_string, 18);
                write(log_fd, buf+offset, 1);
                write(log_fd, "\n", 1);
            }
        }
        if (keyboardRead && !newline)
        {
            if (encrypting)
            {
                if (write(1, unencryptedbuf, 1) == -1)
                {
                    fprintf(stderr, "Error writing: %s\n", strerror(errno));
                    exit(1);
                }
            }
            else
            {
                if (write(1, buf+offset, 1) == -1)
                {
                    fprintf(stderr, "Error writing: %s\n", strerror(errno));
                    exit(1);
                }
            }
            if (write(sockfd, buf+offset, 1) == -1)
            {
                fprintf(stderr, "Error writing: %s\n", strerror(errno));
                exit(1);
            }
            if (logging)
            {
                char sent_string[14] = "SENT   bytes: ";
                sent_string[5] = '0' + bytes;
                write(log_fd, sent_string, 14);
                write(log_fd, buf+offset, 1);
                write(log_fd, "\n", 1);
            }
        }
        offset++;
    }
    exit(0);
}
