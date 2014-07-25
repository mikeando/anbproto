#ifndef ANBPROTO_WORK_QUEUE_H
#define ANBPROTO_WORK_QUEUE_H

typedef struct work_queue_entry work_queue_entry;
typedef struct work_queue work_queue;
void work_queue_create(work_queue **q);
void work_queue_add(work_queue *q, work_queue_entry * entry);

//TODO: These action creators are naf.
work_queue_entry * work_queue_create_action(const char * name);
work_queue_entry * work_queue_create_actionX(const char * name, work_queue * origin);
void work_queue_destroy(work_queue *q);
void work_queue_poison(work_queue *q);

#include <pthread.h>
//TODO: Move this?
struct work_queue_entry {
	//TODO: Not sure this is usefull .. but need some stub filler in there for now.
	const char * name;
};

//TODO: Move this?
struct work_queue {
	pthread_mutex_t lock;
	int poisoned;
	//TODO: This needs a lot of work .. at the moment we just add entries and increment a cursor.
	// And the data is not initialized below yet.
	// Later we'll need to grow/shrink this dynamically.
	int max_entries;
	int write_cursor;
	int read_cursor;
	work_queue_entry** entries;
};

#endif
