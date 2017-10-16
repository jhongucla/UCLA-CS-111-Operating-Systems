// NAME: Justin Hong
// EMAIL: justinjh@ucla.edu
// ID: 604565186

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include "SortedList.h"

int num_threads = 1;
int num_iterations = 1;
int opt_yield = 0;
char sync_opt;
pthread_mutex_t mutex;
SortedList_t* list;
SortedListElement_t* list_elements;
int lock = 0;
int list_len = 0;

void handler(int signum)
{
	if (signum == SIGSEGV)
	{
		fprintf(stderr, "Segmentation fault caught: %s\n", strerror(errno));
		exit(2);
	}
}

void *thread_list(void* tid)
{
	int thread_num = *(int*)tid;
	int i;
	for (i = thread_num; i < num_threads * num_iterations; i += num_threads)
	{
		if (sync_opt == 'm')
		{
			pthread_mutex_lock(&mutex);
			SortedList_insert(list, list_elements+i);
			pthread_mutex_unlock(&mutex);
		}
		else if (sync_opt == 's')
		{
			while (__sync_lock_test_and_set(&lock, 1));
			SortedList_insert(list, list_elements+i);
			__sync_lock_release(&lock);
		}
		else
			SortedList_insert(list, list_elements+i);
	}
	list_len = SortedList_length(list);
	SortedListElement_t* toDelete;
	for (i = thread_num; i < num_threads * num_iterations; i += num_threads)
	{
		if (sync_opt == 'm')
		{
			pthread_mutex_lock(&mutex);
			toDelete = SortedList_lookup(list, list_elements[i].key);
			if (toDelete == NULL)
			{
				fprintf(stderr, "Lookup failed\n");
				exit(2);
			}
			if (SortedList_delete(toDelete) != 0)
			{
				fprintf(stderr, "Delete failed\n");
				exit(2);
			}
			pthread_mutex_unlock(&mutex);
		}
		else if (sync_opt == 's')
		{
			while (__sync_lock_test_and_set(&lock, 1));
			toDelete = SortedList_lookup(list, list_elements[i].key);
			if (toDelete == NULL)
			{
				fprintf(stderr, "Lookup failed\n");
				exit(2);
			}
			if (SortedList_delete(toDelete) != 0)
			{
				fprintf(stderr, "Delete failed\n");
				exit(2);
			}
			__sync_lock_release(&lock);
		}
		else
		{
			toDelete = SortedList_lookup(list, list_elements[i].key);
			if (toDelete == NULL)
			{
				fprintf(stderr, "Lookup failed\n");
				exit(2);
			}
			if (SortedList_delete(toDelete) != 0)
			{
				fprintf(stderr, "Delete failed\n");
				exit(2);
			}
		}
	}
	list_len = SortedList_length(list);
	pthread_exit(NULL);
}

void yield_opts(char* opts)
{
	char yields[3] = {'i', 'd', 'l'};
	int i;
	for (i = 0; *(opts+i) != '\0'; i++)
	{
		if (*(opts+i) == yields[0])
		{
			opt_yield |= INSERT_YIELD;
		}
		else if (*(opts+i) == yields[1])
		{
			opt_yield |= DELETE_YIELD;
		}
		else if (*(opts+i) == yields[2])
		{
			opt_yield |= LOOKUP_YIELD;
		}
		else
		{
			fprintf(stderr, "Incorrect argument for yield option");
			exit(EXIT_FAILURE);
		}
	}
}

int main(int argc, char **argv)
{
	signal(SIGSEGV, handler);
	struct timespec start, end;
	int c;
	while (1)
	{
		static struct option long_options[] =
		{
			{"threads", required_argument, 0, 't'},
			{"iterations", required_argument, 0, 'i'},
			{"yield", required_argument, 0, 'y'},
			{"sync", required_argument, 0, 's'},
			{0, 0, 0, 0}
		};
		c = getopt_long(argc, argv, "t:i:y:s:", long_options, NULL);
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
				yield_opts(optarg);
				break;
			case 's':
				if (strlen(optarg) == 1)
				{
					if (optarg[0] == 'm' || optarg[0] == 's')
					{
						sync_opt = optarg[0];
						break;
					}
				}
				fprintf(stderr, "Incorrect argument for sync option");
				exit(1);
			default:
				fprintf(stderr, "usage: %s --threads=threadnum --iterations=iterationsnum [--yield=yieldarg] [--sync=syncarg]\n", argv[0]);
				exit(1);
		}
	}
	pthread_t threads[num_threads];
	int rc;
	if (sync_opt == 'm')
	{
		rc = pthread_mutex_init(&mutex, NULL);
		if (rc != 0)
		{
			fprintf(stderr, "Error initializing mutex: %s\n", strerror(errno));
			exit(1);
		}
	}
	list = malloc(sizeof(SortedList_t));
	list->key = NULL;
	list->next = list;
	list->prev = list;
	list_elements = malloc(num_threads * num_iterations * sizeof(SortedListElement_t));
	
	srand(time(NULL));
	int i;
	for (i = 0; i < num_threads * num_iterations; i++)
	{
		int length = rand() % 10;
		char* key = malloc((length+1)*sizeof(char));
		int j;
		for (j = 0; j < length; j++)
		{
			int char_index = rand() % 26;
			key[j] = 'a' + char_index;
		}
		key[length] = '\0';
		list_elements[i].key = key;
	}
	
	int tids[num_threads];
	if (clock_gettime(CLOCK_MONOTONIC, &start) == -1)
	{
		fprintf(stderr, "Error starting clock: %s\n", strerror(errno));
		exit(1);
	}
	int t;

	for (t = 0; t < num_threads; t++)
	{
		tids[t] = t;
		rc = pthread_create(&threads[t], NULL, thread_list, (void *)(tids+t));
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

	if (list_len != 0)
	{
		fprintf(stderr, "List length is not zero after all elements have been deleted\n");
		exit(2);
	}

	long total_time = 1000000000 * (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec);
	int num_operations = num_threads * num_iterations * 3;
	long op_time = total_time/num_operations;
	char* syncopts;
	if (sync_opt == 'm')
		syncopts = "m";
	else if (sync_opt == 's')
		syncopts = "s";
	else
		syncopts = "none";
	char* yieldopts;
	if (opt_yield == 0)
		yieldopts = "none";
	else if (opt_yield & INSERT_YIELD && opt_yield & DELETE_YIELD && opt_yield & LOOKUP_YIELD)
		yieldopts = "idl";
	else if (opt_yield & INSERT_YIELD && opt_yield & DELETE_YIELD)
		yieldopts = "id";
	else if (opt_yield & INSERT_YIELD && opt_yield & LOOKUP_YIELD)
		yieldopts = "il";
	else if (opt_yield & DELETE_YIELD && opt_yield & LOOKUP_YIELD)
		yieldopts = "dl";
	else if (opt_yield & INSERT_YIELD)
		yieldopts = "i";
	else if (opt_yield & DELETE_YIELD)
		yieldopts = "d";
	else if (opt_yield & LOOKUP_YIELD)
		yieldopts = "l";
	fprintf(stdout, "list-%s-%s,%d,%d,1,%d,%ld,%ld\n", yieldopts, syncopts, num_threads, num_iterations, num_operations, total_time, op_time);
	free(list_elements);
	free(list);
	exit(0);
}