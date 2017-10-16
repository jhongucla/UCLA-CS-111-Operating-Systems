// NAME: Justin Hong
// EMAIL: justinjh@ucla.edu
// ID: 604565186

#include "mraa.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <poll.h>

const int B = 4275;
const int R0 = 100000;
int interval = 1;
int logging = 0;
char degree_arg = 'F';
int log_fd;
char* logfile;
int off = 0;
int running = 1;
char* line = NULL;
size_t line_size = 0;
int processed = 0;

void handler()
{
	time_t rawtime = time(NULL);
	struct tm* timeinfo = localtime(&rawtime);
	if (logging)
		dprintf(log_fd, "%02d:%02d:%02d SHUTDOWN\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	exit(0);
}

void process_command()
{
	if (getline(&line, &line_size, stdin) == -1)
	{
		fprintf(stderr, "Error reading command: %s\n", strerror(errno));
		exit(1);
	}
	processed = 1;
	if (strcmp(line, "OFF\n") == 0)
		off = 1;
	else if (strcmp(line, "STOP\n") == 0)
		running = 0;
	else if (strcmp(line, "START\n") == 0)
		running = 1;
	else if (strcmp(line, "SCALE=F\n") == 0)
		degree_arg = 'F';
	else if (strcmp(line, "SCALE=C\n") == 0)
		degree_arg = 'C';
	else if (strncmp(line, "PERIOD=", 7) == 0)
		interval = atoi(line+7);
	else if (logging)
	{
		dprintf(log_fd, "Invalid command\n");
		free(line);
		exit(1);
	}
}

int main(int argc, char** argv)
{
    setenv("TZ", "PST8PDT", 1);
    tzset();
	int c;
	while (1)
	{
		static struct option long_options[] =
		{
			{"period", required_argument, 0, 'p'},
			{"scale", required_argument, 0, 's'},
			{"log", required_argument, 0, 'l'},
			{0, 0, 0, 0}
		};
		c = getopt_long(argc, argv, "p:s:l:", long_options, NULL);
		if (c == -1)
			break;
		switch(c)
		{
			case 'p':
				interval = atoi(optarg);
				break;
			case 's':
				if (strlen(optarg) == 1)
				{
					if (optarg[0] == 'F' || optarg[0] == 'C')
					{
						degree_arg = optarg[0];
						break;
					}
				}
				fprintf(stderr, "Incorrect argument for scale option\n");
				exit(1);
			case 'l':
				logging = 1;
				logfile = optarg;
				log_fd = creat(logfile, 0666);
				if (log_fd < 0)
				{
					fprintf(stderr, "Error creating log file: %s\n", strerror(errno));
					exit(1);
				}
				break;
			default:
				fprintf(stderr, "usage: %s [--period=seconds] [--scale=tempformat] [--log=filename]\n", argv[0]);
				exit(1);
		}
	}
	mraa_gpio_context but_d3;
	mraa_aio_context adc_a0;
	float adc_value = 0;
	adc_a0 = mraa_aio_init(0);
	if (adc_a0 == NULL)
		exit(1);
	but_d3 = mraa_gpio_init(3);
	if (but_d3 == NULL)
		exit(1);
	mraa_gpio_dir(but_d3, MRAA_GPIO_IN);
	mraa_gpio_isr(but_d3, MRAA_GPIO_EDGE_RISING, &handler, NULL);
	struct pollfd fds;
	fds.fd = 0;
	fds.events = POLLIN;
	int ready = poll(&fds, 1, 0);
	if (ready > 0 && fds.revents & POLLIN)
		process_command();
	else if (ready == -1)
	{
		fprintf(stderr, "Polling error %s\n", strerror(errno));
		exit(1);
	}
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
			if (processed)
			{
				dprintf(log_fd, "%s", line);
				free(line);
				line = NULL;
				processed = 0;
			}
			if (off)
			{
				dprintf(log_fd, "%02d:%02d:%02d SHUTDOWN\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
				break;
			}
			else if (running)
				dprintf(log_fd, "%02d:%02d:%02d %.1f\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, temp);
		}
		if (off)
			break;
		if (running)
			fprintf(stdout, "%02d:%02d:%02d %.1f\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, temp);
		ready = poll(&fds, 1, 1000*interval);
		if (ready > 0 && fds.revents & POLLIN)
			process_command();
		else if (ready == -1)
		{
			fprintf(stderr, "Polling error: %s\n", strerror(errno));
			exit(1);
		}
	}
	mraa_aio_close(adc_a0);
	mraa_gpio_close(but_d3);
	return MRAA_SUCCESS;
}
