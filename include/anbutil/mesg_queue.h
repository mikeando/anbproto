#ifndef ANBPROTO_MESG_QUEUE
#define ANBPROTO_MESG_QUEUE

#include <pthread.h>
#include "simplemagic.h"
#include "anbproto/structtypes.h"
#include "anbutil/circular_buffer.h"

typedef struct mesg_queue mesg_queue;
typedef struct mesg_queue_entry mesg_queue_entry;
typedef struct mesg_queue_entry_vtable mesg_queue_entry_vtable;

/**
 * A mesg_queue is an inter-thread communication device.
 * Its pretty basic, but is used as the building block
 * for the work_queue that forms the core of the application
 * logic.
 */

struct mesg_queue {
	SMC_ADD_MAGIC();
	pthread_mutex_t lock;
	int poisoned;
  circular_buffer * buffer;
};

/**
 * We use this rather than simple `void*` data, so that the
 * user can implement more complex ownership strategies.
 *
 * Really the only interesting bit of this class is that it's
 * release function is expected to be called by who-ever gets
 * the object off the `mesg_queue`.
 */
struct mesg_queue_entry {
	SMC_ADD_MAGIC();
	mesg_queue_entry_vtable * vtable;
	void * user_data;
};

struct mesg_queue_entry_vtable {
	void (*destroy)(mesg_queue_entry* self);
};

void mesg_queue_create(mesg_queue **q);
void mesg_queue_add(mesg_queue *q, mesg_queue_entry * entry);
void mesg_queue_destroy(mesg_queue *q);
void mesg_queue_poison(mesg_queue *q);

static const int ANBPROTO_QUEUE_DONE  = 0;
static const int ANBPROTO_QUEUE_OK    = 1;
static const int ANBPROTO_QUEUE_EMPTY = 2;
static const int ANBPROTO_QUEUE_BUSY  = 3;

int mesg_queue_take(mesg_queue *q, mesg_queue_entry ** entry);


//TODO: Need to setup the vtable etc.
mesg_queue_entry * mesg_queue_entry_create(void * user_data);
void mesg_queue_entry_destroy(mesg_queue_entry * e);


#endif
