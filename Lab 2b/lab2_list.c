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
pthread_mutex_t* mutexes;
SortedList_t* lists;
SortedListElement_t* list_elements;
int* locks;
int num_lists = 1;
int list_len = 0;
int* list_choices;

void handler(int signum)
{
	if (signum == SIGSEGV)
	{
		fprintf(stderr, "Segmentation fault caught: %s\n", strerror(errno));
		exit(2);
	}
}

int choose_list(const char* key)
{
	int k;
	unsigned int hash = 2166136261u; 
    for (k = 0; key[k] != '\0'; k++)
    {
        hash ^= key[k];
        hash *= 16777619;
    }
    return hash % num_lists;
}

void *thread_list(void* tid)
{
	struct timespec start, end;
	long locking_time = 0;
	int thread_num = *(int*)tid;
	int i;
	for (i = thread_num; i < num_threads * num_iterations; i += num_threads)
	{
		if (sync_opt == 'm')
		{
			if (clock_gettime(CLOCK_MONOTONIC, &start) == -1)
			{
				fprintf(stderr, "Error starting clock: %s\n", strerror(errno));
				exit(1);
			}
			pthread_mutex_lock(mutexes + list_choices[i]);
			if (clock_gettime(CLOCK_MONOTONIC, &end) == -1)
			{
				fprintf(stderr, "Error ending clock: %s\n", strerror(errno));
				exit(1);
			}
			locking_time += 1000000000 * (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec);
			SortedList_insert(lists + list_choices[i], list_elements + i);
			pthread_mutex_unlock(mutexes + list_choices[i]);
		}
		else if (sync_opt == 's')
		{
			if (clock_gettime(CLOCK_MONOTONIC, &start) == -1)
			{
				fprintf(stderr, "Error starting clock: %s\n", strerror(errno));
				exit(1);
			}
			while (__sync_lock_test_and_set(locks + list_choices[i], 1));
			if (clock_gettime(CLOCK_MONOTONIC, &end) == -1)
			{
				fprintf(stderr, "Error ending clock: %s\n", strerror(errno));
				exit(1);
			}
			locking_time += 1000000000 * (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec);
			SortedList_insert(lists + list_choices[i], list_elements + i);
			__sync_lock_release(locks + list_choices[i]);
		}
		else
			SortedList_insert(lists + list_choices[i], list_elements + i);
	}
	list_len = 0;
	int k;
	for (k = 0; k < num_lists; k++)
	{
		if (sync_opt == 'm')
		{
			if (clock_gettime(CLOCK_MONOTONIC, &start) == -1)
			{
				fprintf(stderr, "Error starting clock: %s\n", strerror(errno));
				exit(1);
			}
			pthread_mutex_lock(mutexes + k);
			if (clock_gettime(CLOCK_MONOTONIC, &end) == -1)
			{
				fprintf(stderr, "Error ending clock: %s\n", strerror(errno));
				exit(1);
			}
			locking_time += 1000000000 * (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec);
			list_len += SortedList_length(lists + k);
			pthread_mutex_unlock(mutexes + k);
		}
		else if (sync_opt == 's')
		{
			if (clock_gettime(CLOCK_MONOTONIC, &start) == -1)
			{
				fprintf(stderr, "Error starting clock: %s\n", strerror(errno));
				exit(1);
			}
			while (__sync_lock_test_and_set(locks + k, 1));
			if (clock_gettime(CLOCK_MONOTONIC, &end) == -1)
			{
				fprintf(stderr, "Error ending clock: %s\n", strerror(errno));
				exit(1);
			}
			locking_time += 1000000000 * (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec);
			list_len += SortedList_length(lists + k);
			__sync_lock_release(locks + k);
		}
		else
			list_len += SortedList_length(lists + k);
	}
	SortedListElement_t* toDelete;
	for (i = thread_num; i < num_threads * num_iterations; i += num_threads)
	{
		if (sync_opt == 'm')
		{
			if (clock_gettime(CLOCK_MONOTONIC, &start) == -1)
			{
				fprintf(stderr, "Error starting clock: %s\n", strerror(errno));
				exit(1);
			}
			pthread_mutex_lock(mutexes + list_choices[i]);
			if (clock_gettime(CLOCK_MONOTONIC, &end) == -1)
			{
				fprintf(stderr, "Error ending clock: %s\n", strerror(errno));
				exit(1);
			}
			locking_time += 1000000000 * (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec);
			toDelete = SortedList_lookup(lists + list_choices[i], list_elements[i].key);
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
			pthread_mutex_unlock(mutexes + list_choices[i]);
		}
		else if (sync_opt == 's')
		{
			if (clock_gettime(CLOCK_MONOTONIC, &start) == -1)
			{
				fprintf(stderr, "Error starting clock: %s\n", strerror(errno));
				exit(1);
			}
			while (__sync_lock_test_and_set(locks + list_choices[i], 1));
			if (clock_gettime(CLOCK_MONOTONIC, &end) == -1)
			{
				fprintf(stderr, "Error ending clock: %s\n", strerror(errno));
				exit(1);
			}
			locking_time += 1000000000 * (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec);
			toDelete = SortedList_lookup(lists + list_choices[i], list_elements[i].key);
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
			__sync_lock_release(locks + list_choices[i]);
		}
		else
		{
			toDelete = SortedList_lookup(lists + list_choices[i], list_elements[i].key);
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
	return (void *)locking_time;
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
			{"lists", required_argument, 0, 'l'},
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
			case 'l':
				num_lists = atoi(optarg);
				break;
			default:
				fprintf(stderr, "usage: %s --threads=threadnum --iterations=iterationsnum [--yield=yieldarg] [--sync=syncarg] [--lists=listsnum]\n", argv[0]);
				exit(1);
		}
	}
	pthread_t threads[num_threads];
	int rc;
	if (sync_opt == 'm')
	{
		mutexes = malloc(num_lists * sizeof(pthread_mutex_t));
		int i;
		for (i = 0; i < num_lists; i++)
		{
			rc = pthread_mutex_init(mutexes+i, NULL);
			if (rc != 0)
			{
				fprintf(stderr, "Error initializing mutex: %s\n", strerror(errno));
				exit(1);
			}
		}
	}
	else if (sync_opt == 's')
	{
		locks = malloc(num_lists * sizeof(int));
		int i;
		for (i = 0; i < num_lists; i++)
			locks[i] = 0;
	}
	
	lists = malloc(num_lists * sizeof(SortedList_t));
	int j;
	for (j = 0; j < num_lists; j++)
	{
		lists[j].key = NULL;
		lists[j].next = lists + j;
		lists[j].prev = lists + j;
	}
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
	
	list_choices = malloc(num_threads * num_iterations * sizeof(int));
	for (i = 0; i < num_threads * num_iterations; i++)
		list_choices[i] = choose_list(list_elements[i].key);

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
	long total_locking_time = 0;
	for (t = 0; t < num_threads; t++)
	{
		void* thread_locking_time;
		rc = pthread_join(threads[t], &thread_locking_time);
		if (rc)
		{
			fprintf(stderr, "Error joined thread: %s\n", strerror(errno));
			exit(1);
		}
		total_locking_time += (long)thread_locking_time;
	}
	if (clock_gettime(CLOCK_MONOTONIC, &end) == -1)
	{
		fprintf(stderr, "Error ending clock: %s\n", strerror(errno));
		exit(1);
	}

	list_len = 0;
	int k;
	for (k = 0; k < num_lists; k++)
		list_len += SortedList_length(lists+k);
	if (list_len != 0)
	{
		fprintf(stderr, "List length is not zero after all elements have been deleted\n");
		exit(2);
	}

	long total_time = 1000000000 * (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec);
	int num_operations = num_threads * num_iterations * 3;
	long op_time = total_time/num_operations;
	long avg_lock_wait = 0;
	if (sync_opt == 'm' || sync_opt == 's')
		avg_lock_wait = total_locking_time/(num_threads*num_iterations*2);
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
	fprintf(stdout, "list-%s-%s,%d,%d,%d,%d,%ld,%ld,%ld\n", yieldopts, syncopts, num_threads, num_iterations, num_lists, num_operations, total_time, op_time, avg_lock_wait);
	free(list_elements);
	free(lists);
	free(mutexes);
	exit(0);
}
