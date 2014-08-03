#ifndef ANBPROTO_WORK_QUEUE_H
#define ANBPROTO_WORK_QUEUE_H

typedef struct work_queue_entry work_queue_entry;
typedef struct work_queue work_queue;
typedef struct worker worker;
void work_queue_create(work_queue **q);
void work_queue_add(work_queue *q, work_queue_entry * entry);

//TODO: This action creator is naf.
work_queue_entry * work_queue_create_action(const char * name, int type, void(*process)(work_queue_entry* self, worker* w), void* user_data);
void work_queue_destroy(work_queue *q);
void work_queue_poison(work_queue *q);

#include <pthread.h>
#include <stdint.h>

//TODO: Make these more type-safey
#define ANBP_DB_SAVE_OBJECT 1
#define ANBP_DB_LOAD_OBJECT 2

struct work_queue_entry_vtable {
	void(*process)(work_queue_entry* self, worker * w);
};

typedef struct work_queue_entry_vtable work_queue_entry_vtable;

struct work_queue_entry {
	uint64_t magic;
	__attribute__((deprecated)) const char * name;
	__attribute__((deprecated)) int type;
	work_queue_entry_vtable * vtable;
	void * user_data;
};

//TODO: Move this?
struct work_queue {
	uint64_t magic;
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
