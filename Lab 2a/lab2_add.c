// NAME: Justin Hong
// EMAIL: justinjh@ucla.edu
// ID: 604565186

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

int num_threads = 1;
int num_iterations = 1;
int opt_yield = 0;
char sync_opt;
pthread_mutex_t mutex;
int lock = 0;

void add(long long *pointer, long long value)
{
	long long sum = *pointer + value;
	if (opt_yield)
		sched_yield();
	*pointer = sum;
}

void add_comp(long long *pointer, long long value)
{
	long long count;
	long long new_count;
	count = *pointer;
	new_count = count + value;
	if (opt_yield)
		sched_yield();
	while (__sync_val_compare_and_swap(pointer, count, new_count) != count)
	{
		count = *pointer;
		new_count = count + value;
		if (opt_yield)
			sched_yield();
	}
}

void *thread_add(void *thread_arg)
{
	long long *counter;
	counter = (long long *)thread_arg;
	int i;
	for (i = 0; i < num_iterations; i++)
	{
		if (sync_opt == 'm')
		{
			pthread_mutex_lock(&mutex);
			add(counter, 1);
			pthread_mutex_unlock(&mutex);
		}
		else if (sync_opt == 's')
		{
			while (__sync_lock_test_and_set(&lock, 1));
			add(counter, 1);
			__sync_lock_release(&lock);
		}
		else if (sync_opt == 'c')
			add_comp(counter, 1);
		else
			add(counter, 1);
	}
	for (i = 0; i < num_iterations; i++)
	{
		if (sync_opt == 'm')
		{
			pthread_mutex_lock(&mutex);
			add(counter, -1);
			pthread_mutex_unlock(&mutex);
		}
		else if (sync_opt == 's')
		{
			while (__sync_lock_test_and_set(&lock, 1));
			add(counter, -1);
			__sync_lock_release(&lock);
		}
		else if (sync_opt == 'c')
			add_comp(counter, -1);
		else
			add(counter, -1);
	}
	pthread_exit(NULL);
}

int main(int argc, char **argv)
{
	struct timespec start, end;
	int c;
	while (1)
	{
		static struct option long_options[] =
		{
			{"threads", required_argument, 0, 't'},
			{"iterations", required_argument, 0, 'i'},
			{"yield", no_argument, 0, 'y'},
			{"sync", required_argument, 0, 's'},
			{0, 0, 0, 0}
		};
		c = getopt_long(argc, argv, "t:i:ys:", long_options, NULL);
		if (c == -1)
			break;
		switch(c)
		{
			case 't':
				num_threads = atoi(optarg);
				break;
			case 'i':
				num_iterations = atoi(optarg);
				break;
			case 'y':
				opt_yield = 1;
				break;
			case 's':
				if (strlen(optarg) == 1)
				{
					if (optarg[0] == 'm' || optarg[0] == 's' || optarg[0] == 'c')
					{
						sync_opt = optarg[0];
						break;
					}
				}
				fprintf(stderr, "Incorrect argument for sync option\n");
				exit(1);
			default:
				fprintf(stderr, "usage: %s --threads=threadnum --iterations=iterationsnum [--yield] [--sync=syncarg]\n", argv[0]);
				exit(1);
		}
	}
	pthread_t threads[num_threads];
	int rc;
	int t;
	long long counter = 0;
	if (sync_opt == 'm')
	{
		rc = pthread_mutex_init(&mutex, NULL);
		if (rc != 0)
		{
			fprintf(stderr, "Error initializing mutex: %s\n", strerror(errno));
			exit(1);
		}
	}

	if (clock_gettime(CLOCK_MONOTONIC, &start) == -1)
	{
		fprintf(stderr, "Error starting clock: %s\n", strerror(errno));
		exit(1);
	}

	for (t = 0; t < num_threads; t++)
	{
		rc = pthread_create(&threads[t], NULL, thread_add, (void *) &counter);
		if (rc)
		{
			fprintf(stderr, "Error creating thread: %s\n", strerror(errno));
			exit(1);
		}
	}

	for (t = 0; t < num_threads; t++)
	{
		rc = pthread_join(threads[t], NULL);
		if (rc)
		{
			fprintf(stderr, "Error joined thread: %s\n", strerror(errno));
			exit(1);
		}
	}
	if (clock_gettime(CLOCK_MONOTONIC, &end) == -1)
	{
		fprintf(stderr, "Error ending clock: %s\n", strerror(errno));
		exit(1);
	}
	long total_time = 1000000000 * (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec);
	int num_operations = num_threads * num_iterations * 2;
	long op_time = total_time/num_operations;
	char* output;
	if (opt_yield)
	{
		if (sync_opt == 'm')
			output = "add-yield-m";
		else if (sync_opt == 's')
			output = "add-yield-s";
		else if (sync_opt == 'c')
			output = "add-yield-c";
		else
			output = "add-yield-none"; 
	}
	else
	{
		if (sync_opt == 'm')
			output = "add-m";
		else if (sync_opt == 's')
			output = "add-s";
		else if (sync_opt == 'c')
			output = "add-c";
		else
			output = "add-none"; 
	}
	fprintf(stdout, "%s,%d,%d,%d,%ld,%ld,%lld\n", output, num_threads, num_iterations, num_operations, total_time, op_time, counter);
	exit(0);
}