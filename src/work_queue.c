#include "anbproto/work_queue.h"
#include "anbproto/structtypes.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

void work_queue_create(work_queue **q) {
	work_queue * retval = malloc(sizeof(work_queue));
	smc_init_magic(work_queue, retval);
	mesg_queue_create(&retval->mesg_queue);
	*q = retval;
};

void work_queue_destroy(work_queue *q) {
	mesg_queue_destroy(q->mesg_queue);
	free(q);
};

void work_queue_poison(work_queue *q) {
	mesg_queue_poison(q->mesg_queue);
};

void work_queue_add(work_queue * q, work_queue_entry * entry ) {
	mesg_queue_entry * e = malloc(sizeof(mesg_queue_entry));
	smc_init_magic(mesg_queue_entry, e);
	e->user_data = entry;

	mesg_queue_add(q->mesg_queue, e);
}

int work_queue_take(work_queue *q, work_queue_entry ** entry) {

	mesg_queue_entry *e;
	int ok = mesg_queue_take(q->mesg_queue, &e);
	if(ok!=ANBPROTO_QUEUE_OK)
		return ok;

	work_queue_entry * wqe = e->user_data;
	free(e);
	*entry = wqe;
	return ANBPROTO_QUEUE_OK ;
}



//TODO: These action creators are naf.
work_queue_entry * work_queue_create_action(const char * name, int type, void(*process)(work_queue_entry* self, worker* w), void* user_data) {
	assert(process!=NULL);
	work_queue_entry * retval = malloc(sizeof(work_queue_entry));
	retval->vtable = malloc(sizeof(work_queue_entry_vtable));
	retval->name = name;
	retval->type = type;
	retval->vtable->process = process;
	retval->user_data = user_data;
	return retval;
}
