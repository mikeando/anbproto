#include "anbproto/work_queue.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>



void work_queue_create(work_queue **q) {
	work_queue * retval = malloc(sizeof(work_queue));
	memset(retval, 0, sizeof(work_queue));
	retval->poisoned = 0;
	pthread_mutex_init(&retval->lock, NULL);

	retval->entries = malloc(sizeof(work_queue_entry*)*10);
	retval->max_entries = 10;
	retval->read_cursor = 0;  // Where the next read will read from
	retval->write_cursor = 0; // Where the next add will write to
	*q = retval;
};

void work_queue_destroy(work_queue *q) {
	pthread_mutex_destroy(&q->lock);
	free(q->entries);
	free(q);
};

void work_queue_poison(work_queue *q) {
	pthread_mutex_lock(&q->lock);
	q->poisoned = 1;
	pthread_mutex_unlock(&q->lock);
};

void work_queue_add(work_queue * q, work_queue_entry * entry ) {
	//TODO: Report errors better
	if(q->write_cursor >= q->max_entries-1 ) {
		printf("OMG I IZ TOAST.\n");
		return;
	}

	q->entries[q->write_cursor] = entry;
	q->write_cursor++;
}



//TODO: These action creators are naf.
work_queue_entry * work_queue_create_action(const char * name) {
	work_queue_entry * retval = malloc(sizeof(work_queue_entry));
	retval->name = name;
	return retval;
}

work_queue_entry * work_queue_create_actionX(const char * name, work_queue * origin) {
	work_queue_entry * retval = malloc(sizeof(work_queue_entry));
	retval->name = name;
	return retval;
}

