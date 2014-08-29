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

  circular_buffer_create(&retval->buffer, 10);
	*q = retval;
};

void mesg_queue_destroy(mesg_queue * q) {
	pthread_mutex_destroy(&q->lock);
  uint32_t queued_entries = circular_buffer_size(q->buffer);
  //TODO: Need better handling of the still queued entries
  if(queued_entries>0) {
    printf("%d entries still queued - unqueueing\n", (int)queued_entries);
    for(int i=0; i<queued_entries; ++i) {
      void * dummy=NULL;
      circular_buffer_get(q->buffer, &dummy);
      printf("unqueued %p\n", dummy);
    }
  }
  circular_buffer_free(q->buffer);
	free(q);
}

void mesg_queue_poison(mesg_queue *q) {
	pthread_mutex_lock(&q->lock);
	q->poisoned = 1;
	pthread_mutex_unlock(&q->lock);
}

void mesg_queue_add(mesg_queue *q, mesg_queue_entry * entry) {
  pthread_mutex_lock(&q->lock);
  circular_buffer_put(q->buffer,entry);
  pthread_mutex_unlock(&q->lock);
}

#include <assert.h>
int mesg_queue_take(mesg_queue *q, mesg_queue_entry ** entry) {
	if(pthread_mutex_trylock(&q->lock)!=0)
		return ANBPROTO_QUEUE_BUSY;


  void* result;
  int ok = circular_buffer_get(q->buffer, &result);

  if(ok==ECIRC_OK) {
    *entry = (mesg_queue_entry*)result;
    pthread_mutex_unlock(&q->lock);
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

