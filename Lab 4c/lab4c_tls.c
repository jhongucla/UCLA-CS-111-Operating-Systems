// NAME: Justin Hong
// EMAIL: justinjh@ucla.edu
// ID: 604565186

#include "mraa.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <poll.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

int id = 0;
char* host = NULL;
int log_fd;
int sockfd;
int logging = 0;
int port_no = -1;
const int B = 4275;
const int R0 = 100000;
int interval = 1;
char degree_arg = 'F';
int off = 0;
int running = 1;
char* line = NULL;
size_t line_size = 0;
int processed = 0;
FILE* fd;
const SSL_METHOD *method;
SSL_CTX *ctx;
SSL *ssl;

void *reading(void *thread_arg)
{
	while(1)
	{
		free(line);
		line = malloc(sizeof(char) * 100);
		if (SSL_read(ssl, line, 20) > 0)
		{
			if (strncmp(line, "OFF", 3) == 0)
			{
				off = 1;
				dprintf(log_fd, "OFF\n");
				break;
			}
			else if (strncmp(line, "STOP", 4) == 0)
			{
				running = 0;
				dprintf(log_fd, "STOP\n");
			}
			else if (strncmp(line, "START", 5) == 0)
			{
				running = 1;
				dprintf(log_fd, "START\n");
			}
			else if (strncmp(line, "SCALE=F", 7) == 0)
			{
				degree_arg = 'F';
				dprintf(log_fd, "SCALE=F\n");
			}
			else if (strncmp(line, "SCALE=C", 7) == 0)
			{
				degree_arg = 'C';
				dprintf(log_fd, "SCALE=C\n");
			}
			else if (strncmp(line, "PERIOD=", 7) == 0)
			{
				interval = atoi(line+7);
				dprintf(log_fd, "PERIOD=%d\n", interval);
			}
			else if (logging)
			{
				dprintf(log_fd, "Invalid command\n");
				exit(2);
			}
		}
	}
}

int main(int argc, char **argv)
{
	line = malloc(sizeof(char) * 10);
	int c;
	char* log_file = NULL;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	while (optind < argc)
	{
		static struct option long_options[] = 
		{
			{"id", required_argument, 0, 'i'},
			{"host", required_argument, 0, 'h'},
			{"log", required_argument, 0, 'l'},
			{0, 0, 0, 0}
		};
		c = getopt_long(argc, argv, "i:h:l:", long_options, NULL);
		if (c != -1)
		{
			switch(c)
			{
				case 'i':
					id = atoi(optarg);
					break;
				case 'h':
					host = optarg;
					break;
				case 'l':
					logging = 1;
					log_file = optarg;
					log_fd = creat(log_file, 0666);
					if (log_fd == -1)
					{
						fprintf(stderr, "Error creating log file: %s\n", strerror(errno));
						exit(1);
					}
					break;
				default:
				fprintf(stderr, "Usage: %s port_number [--id=id number] [--host=name or address] [--log=filename]\n", argv[0]);
				exit(1);
			}
		}
		else
		{
			port_no = atoi(argv[optind]);
			optind++;
		}
	}
	if (port_no == -1)
	{
		fprintf(stderr, "Missing required port number\n");
		exit(1);
	}
    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        fprintf(stderr, "Error opening socket: %s\n", strerror(errno));
        exit(2);
    }
    server = gethostbyname(host);
    if (server == NULL)
    {
        fprintf(stderr, "Error finding host: %s\n", strerror(errno));
        exit(1);
    }
    bzero((char*) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char*) server->h_addr, (char*) &serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(port_no);
    if (connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
    {
        fprintf(stderr, "Error connecting: %s\n", strerror(errno));
        exit(2);
    }
    
    SSL_load_error_strings();
    SSL_library_init();
    OpenSSL_add_all_algorithms();

    method = SSLv23_client_method();
    ctx = SSL_CTX_new(method);
    if (ctx == NULL)
    {
    	fprintf(stderr, "Error creating SSL context structure\n");
    	exit(2);
    }
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sockfd);

    if (SSL_connect(ssl) == -1)
    {
    	fprintf(stderr, "Error creating TLS connection\n");
    	exit(2);
    }
	mraa_aio_context adc_a0;
	float adc_value = 0;
	adc_a0 = mraa_aio_init(0);
	if (adc_a0 == NULL)
		exit(2);

	setenv("TZ", "PST8PDT", 1);
    tzset();

    char buf[1024];
	sprintf(buf, "ID=%d\n", id);
	SSL_write(ssl, buf, strlen(buf));

	pthread_t thread;
	int rc = pthread_create(&thread, NULL, reading, NULL);
	if (rc)
	{
		fprintf(stderr, "Error creating thread: %s\n", strerror(errno));
		exit(2);
	}
	line = malloc(sizeof(char) * 100);
	while(1)
	{
		adc_value = mraa_aio_read(adc_a0);
		float R = 1023.0/adc_value-1.0;
		R = R0*R;
		float temp = 1.0/(log(R/R0)/B+1/298.15)-273.15;
		if (degree_arg == 'F')
			temp = temp*1.8 + 32;
		time_t rawtime = time(NULL);
		struct tm* timeinfo = localtime(&rawtime);
		if (logging)
		{
			if (off)
			{
				dprintf(log_fd, "%02d:%02d:%02d SHUTDOWN\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
			}
			else if (running)
				dprintf(log_fd, "%02d:%02d:%02d %.1f\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, temp);
		}
		if (off)
		{	
			sprintf(buf, "%02d:%02d:%02d SHUTDOWN\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
			SSL_write(ssl, buf, strlen(buf));
			break;
		}
		if (running)
		{
			sprintf(buf, "%02d:%02d:%02d %.1f\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, temp);
			SSL_write(ssl, buf, strlen(buf));
		}
		if (running)
			usleep(interval*1000000);
	}
	rc = pthread_join(thread, NULL);
	if (rc)
	{
		fprintf(stderr, "Error joining threads: %s\n", strerror(errno));
		exit(2);
	}
	mraa_aio_close(adc_a0);
	close(sockfd);
	SSL_free(ssl);
    close(sockfd);
    SSL_CTX_free(ctx);
	return MRAA_SUCCESS;
}
