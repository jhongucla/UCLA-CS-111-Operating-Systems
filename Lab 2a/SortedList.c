// NAME: Justin Hong
// EMAIL: justinjh@ucla.edu
// ID: 604565186

#define _GNU_SOURCE
#include "SortedList.h"
#include <pthread.h>
#include <string.h>

void SortedList_insert(SortedList_t *list, SortedListElement_t *element)
{
	if (list == NULL || element == NULL)
		return;
	SortedListElement_t *curr_ele = list->next;
	while (curr_ele != list && strcmp(element->key, curr_ele->key) > 0)
		curr_ele = curr_ele->next;
	if (opt_yield & INSERT_YIELD)
		sched_yield();
	element->prev = curr_ele->prev;
	element->next = curr_ele;
	curr_ele->prev->next = element;
	curr_ele->prev = element;
}

int SortedList_delete(SortedListElement_t *element)
{
	if (element == NULL)
		return 1;
	if (element->next->prev == element->prev->next)
	{
		if (opt_yield & DELETE_YIELD)
			sched_yield();
		element->prev->next = element->next;
		element->next->prev = element->prev;
		return 0;
	}
	return 1;
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key)
{
	if (list == NULL || key == NULL)
		return NULL;
	SortedListElement_t *curr_ele = list->next;
	while (curr_ele != list)
	{
		if (strcmp(curr_ele->key, key) == 0)
			return curr_ele;
		if (opt_yield & LOOKUP_YIELD)
			sched_yield();
		curr_ele = curr_ele->next;
	}
	return NULL;
}

int SortedList_length(SortedList_t *list)
{
	if (list == NULL)
		return -1;
	int len = 0;
	SortedListElement_t *curr_ele = list->next;
	while (curr_ele != list)
	{
		len++;
		if (opt_yield & LOOKUP_YIELD)
			sched_yield();
		curr_ele = curr_ele->next;
	}
	return len;
}
