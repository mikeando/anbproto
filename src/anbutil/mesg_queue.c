#include "anbutil/mesg_queue.h"
#include "anbutil/structtypes.h"
#include "simplemagic.h"
#include <stdlib.h>
#include <string.h>

//TODO: Remove me when better error handling implemented
#include <stdio.h>

void mesg_queue_create(mesg_queue **q) {
	mesg_queue * retval = malloc(sizeof(mesg_queue));
	memset(retval, 0, sizeof(mesg_queue));
	smc_init_magic(mesg_queue, retval);
	retval->poisoned = 0;
	pthread_mutex_init(&retval->lock, NULL);

	retval->entries = malloc(sizeof(mesg_queue_entry*)*10);
	retval->max_entries = 10;
	retval->read_cursor = 0;  // Where the next read will read from
	retval->write_cursor = 0; // Where the next add will write to
	*q = retval;
};

void mesg_queue_destroy(mesg_queue * q) {
	pthread_mutex_destroy(&q->lock);
	free(q->entries);
	free(q);
}

void mesg_queue_poison(mesg_queue *q) {
	pthread_mutex_lock(&q->lock);
	q->poisoned = 1;
	pthread_mutex_unlock(&q->lock);
}

void mesg_queue_add(mesg_queue *q, mesg_queue_entry * entry) {
	//TODO: Report errors better
	if(q->write_cursor >= q->max_entries-1 ) {
		printf("OMG I IZ TOAST.\n");
		return;
	}

	q->entries[q->write_cursor] = entry;
	q->write_cursor++;
}

#include <assert.h>
int mesg_queue_take(mesg_queue *q, mesg_queue_entry ** entry) {
	if(pthread_mutex_trylock(&q->lock)!=0)
		return ANBPROTO_QUEUE_BUSY;


	if(q->read_cursor < q->write_cursor) {
                mesg_queue_entry * e = q->entries[q->read_cursor];
                q->read_cursor++;
                pthread_mutex_unlock(&q->lock);

		*entry = e;
		return ANBPROTO_QUEUE_OK;
	}

	if(q->poisoned==1) {
		pthread_mutex_unlock(&q->lock);
		return ANBPROTO_QUEUE_DONE;
	}

	pthread_mutex_unlock(&q->lock);
	return ANBPROTO_QUEUE_EMPTY;
}

mesg_queue_entry * mesg_queue_entry_create(void * user_data) {
    mesg_queue_entry * e = malloc(sizeof(mesg_queue_entry));
    smc_init_magic(mesg_queue_entry, e);
    e->user_data = user_data;
    e->vtable = NULL;
    return e;
}

//TODO: Use the vtable if given
void mesg_queue_entry_destroy(mesg_queue_entry * e) {
    free(e);
}

