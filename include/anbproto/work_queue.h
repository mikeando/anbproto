#ifndef ANBPROTO_WORK_QUEUE_H
#define ANBPROTO_WORK_QUEUE_H

#include "anbproto/mesg_queue.h"

/**
 * A `work_queue` is esentially a `mesg_queue` that contains
 * `work_queue_entry` objects. It is expected to be running
 * inside some kind of work loop in a single thread , and
 * processing its entries one by one.
 */

typedef struct work_queue_entry work_queue_entry;
typedef struct work_queue work_queue;
typedef struct worker worker;
void work_queue_create(work_queue **q);
void work_queue_add(work_queue *q, work_queue_entry * entry);

int work_queue_take(work_queue *q, work_queue_entry ** entry);

//TODO: This action creator is naf.
work_queue_entry * work_queue_create_action(const char * name, int type, void(*process)(work_queue_entry* self, worker* w), void* user_data);
void work_queue_destroy(work_queue *q);
void work_queue_poison(work_queue *q);

#include <stdint.h>
#include "simplemagic.h"
#include "anbproto/structtypes.h"

// TODO: Make these more type-safey
// TODO: Remove these!
#define ANBP_DB_SAVE_OBJECT 1
#define ANBP_DB_LOAD_OBJECT 2

struct work_queue_entry_vtable {
	void(*process)(work_queue_entry* self, worker * w);
};

typedef struct work_queue_entry_vtable work_queue_entry_vtable;

struct work_queue_entry {
	SMC_ADD_MAGIC();
	const char * name;
	int type;
	work_queue_entry_vtable * vtable;
	void * user_data;
};

//TODO: Move this?
struct work_queue {
	SMC_ADD_MAGIC();
	mesg_queue * mesg_queue;
};

//TODO: Not sure anything below here belongs in here
#include "anbproto/logger.h"
struct worker_vtable {
	SMC_ADD_MAGIC();
	void (*idle)(worker * );
	void (*process)(worker * ,work_queue_entry* entry);
};

struct worker {
	SMC_ADD_MAGIC();
	struct worker_vtable * vtable;
	work_queue * queue;
	const char * name;
	logger * root_logger;
	logger * logger;
	void * user_data;
};

void run_worker( struct worker * w );

#endif
