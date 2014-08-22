#include "simple_odb.h"

#include "anbproto/work_queue.h"
#include <assert.h>
#include <stdlib.h>

#define SIMPLE_ODB_GET_IDS 1001
#define SIMPLE_ODB_GET_METAS 1002
#define SIMPLE_ODB_GET_OBJECTS 1003
#define SIMPLE_ODB_GET_OBJECT 1004
#define SIMPLE_ODB_PUT_OBJECT 1005

typedef struct simple_odb_thread_data simple_odb_thread_data;
struct simple_odb {
    SMC_ADD_MAGIC();
    work_queue * q;

    //TODO: Move these into a ref counted structure so we can create more than
    //one simple_odb object?
    simple_odb_thread_data * simple_odb_data;
    pthread_t * simple_odb_thread;
};

typedef struct simple_odb simple_odb;

//TODO: How do we report errors?
#include <stdio.h>
void object_save(anbp_object* obj);

void object_load(anbp_object_id *id, anbp_object** result) {
    //TODO: Implement me
    printf("object_load: Loading object...\n");

    //TODO: Needs a bunch more checking!
    //TODO: Make the root be part of the simple_odb config.
	const char * home = getenv("HOME");
	char path[1024];
	snprintf(path,1024,"%s/anbproto/objects/%s", home, id->id);
    FILE* f = fopen(path,"rb");
    //TODO: Better error handling
    if(f==NULL) {
        printf("Loading object %s failed\n", path);
        exit(1);
    }
    //TODO: Remove this field its now unused
    int dumb_id;
    fread(&dumb_id, sizeof(int), 1, f);
    int dumb_counter;
    fread(&dumb_counter, sizeof(int), 1, f);
    int dumb_mesg_len;
    fread(&dumb_mesg_len, sizeof(int), 1, f);
    char * dumb_mesg = malloc(dumb_mesg_len+1);
    fread(dumb_mesg, dumb_mesg_len, 1, f);
    dumb_mesg[dumb_mesg_len]=0;
    fclose(f);
    //TODO: Copy the object id into the object - note id inside the object is wrong

    anbp_object_create(result, id, dumb_counter, dumb_mesg); 
    free(dumb_mesg);
}

//TODO: Move the object id into the object - note current id inside the object is wrong
void object_save(anbp_object* obj) {
    printf("object_save: Saving object...\n");

    //TODO: Needs a bunch more checking!
    //TODO: Make the root be part of the simple_odb config.
	const char * home = getenv("HOME");
	char path[1024];
	snprintf(path,1024,"%s/anbproto/objects/%s", home, obj->id->id);
    FILE* f = fopen(path,"wb");
    //TODO: Better error handling
    if(f==NULL) {
        printf("Saving object %s failed\n", path);
        exit(1);
    }
    //TODO: Remove this unneeded field.
    int dumb_id = 0;
    fwrite(&dumb_id, sizeof(int), 1, f);
    fwrite(&(obj->counter), sizeof(int), 1, f);
    int mesg_len = strlen(obj->mesg);
    fwrite(&mesg_len, sizeof(int), 1, f);
    fwrite(obj->mesg, mesg_len, 1, f);
    fclose(f);
}

void simple_db_cb_get_object(worker * w, work_queue_entry * e, req_odb_get_object * req) {
    message(w->logger, "  + gotta go fetch me object with id %s\n", req->id.id);

    // TODO: Handle errors here
    // TODO: Should this be behind another layer?
    object_load(&req->id, &req->result);

    req->done(req);
}

void simple_db_callback(work_queue_entry* self, worker* w) {
	message(w->logger, "in simple_db_callback...\n");
    message(w->logger, "  +  self.name = %s\n", self->name);
    message(w->logger, "  +  self.type = %d\n", self->type);
    message(w->logger, "  +  self.vtable = %p\n", self->vtable);
    message(w->logger, "  +  self.user_data = %p\n", self->user_data);

    if(self->type==SIMPLE_ODB_GET_OBJECT) {
        message(w->logger, "looks like a Get object call - forwarding\n");
        smc_check_type(req_odb_get_object, self->user_data);
        simple_db_cb_get_object(w, self, (req_odb_get_object*)self->user_data);
    }

    //TODO: Implement other request types.
}


int simple_odb_get_ids(odb * self, req_odb_get_ids * req ) {
    smc_check_type(simple_odb, self->impl);
    simple_odb * sodb = self->impl;
    work_queue_add(sodb->q, work_queue_create_action("Get IDs", SIMPLE_ODB_GET_IDS, simple_db_callback, req));
    return 0;
}
int simple_odb_get_metas( odb * self, req_odb_get_metas * req ) {
    smc_check_type(simple_odb, self->impl);
    simple_odb * sodb = self->impl;
    work_queue_add(sodb->q, work_queue_create_action("Get metas", SIMPLE_ODB_GET_METAS, simple_db_callback, req));
    return 0;
}
int simple_odb_get_objects( odb * self, req_odb_get_objects * req ) {
    smc_check_type(simple_odb, self->impl);
    simple_odb * sodb = self->impl;
    work_queue_add(sodb->q, work_queue_create_action("Get objects", SIMPLE_ODB_GET_OBJECTS, simple_db_callback, req));
    return 0;
}
int simple_odb_get_object( odb * self, req_odb_get_object * req ) {
    smc_check_type(simple_odb, self->impl);
    simple_odb * sodb = self->impl;
    work_queue_add(sodb->q, work_queue_create_action("Get object", SIMPLE_ODB_GET_OBJECT, simple_db_callback, req));
    return 0;
}
int simple_odb_put_object( odb * self, req_odb_put_object * req ) {
    smc_check_type(simple_odb, self->impl);
    simple_odb * sodb = self->impl;
    work_queue_add(sodb->q, work_queue_create_action("Put object", SIMPLE_ODB_PUT_OBJECT, simple_db_callback, req));
    return 0;
}

struct simple_odb_thread_data {
    SMC_ADD_MAGIC();
    work_queue * queue;
    logger * root_logger;
};

void simple_odb_thread_idle(worker * w) {
	message(w->logger, "idling...\n");
}

void simple_odb_thread_process( worker * w, work_queue_entry * entry ) {
	message(w->logger,"My entry is : %s\n", entry->name);
	assert(entry->vtable!=NULL);
	entry->vtable->process(entry, w);
}

void* simple_odb_thread_fn(void * data) {
	smc_check_type(simple_odb_thread_data, data);
	simple_odb_thread_data * d = data;

    // TODO: Change the idle / process functions here?
    // TODO: Pass extra info in the worker?
	struct worker_vtable vtable = {smc__magic_worker_vtable, &simple_odb_thread_idle, &simple_odb_thread_process};
	struct worker w = { smc__magic_worker, &vtable, d->queue, "SODB", d->root_logger, NULL, NULL };
	run_worker(&w);
	return NULL;
}

odb * simple_odb_init( logger * root_logger ) {

    // Create the queue
	work_queue * odb_queue;
	work_queue_create(&odb_queue);

    simple_odb_thread_data * thread_data = malloc(sizeof(simple_odb_thread_data));
    smc_init_magic(simple_odb_thread_data, thread_data);

    thread_data->queue = odb_queue;
    thread_data->root_logger = root_logger;

    pthread_t * thread = malloc(sizeof(pthread_t));

    // Start the worker thread
	pthread_create(thread, NULL, &simple_odb_thread_fn, thread_data);

    odb * retval = malloc(sizeof(odb));
    smc_init_magic(odb, retval);
    retval->vtable = malloc(sizeof(odb_vtable));
    retval->vtable->get_ids = &simple_odb_get_ids;
    retval->vtable->get_metas = &simple_odb_get_metas;
    retval->vtable->get_object = &simple_odb_get_object;
    retval->vtable->get_objects = &simple_odb_get_objects;
    retval->vtable->put_object = &simple_odb_put_object;
    simple_odb * sodb = malloc(sizeof(simple_odb));
    smc_init_magic(simple_odb, sodb);
    sodb->q = odb_queue;
    sodb->simple_odb_data = thread_data;
    sodb->simple_odb_thread = thread;
    retval->impl = sodb;
    return retval;
}

void simple_odb_destroy(odb* db) {
    assert(db!=NULL);

    smc_check_type(odb,db);
    smc_check_type(simple_odb, db->impl);
    simple_odb* sodb = (simple_odb*)db->impl;

    work_queue_poison(sodb->q);
    pthread_join(*sodb->simple_odb_thread, NULL);
    free(sodb->simple_odb_thread);
    free(sodb->simple_odb_data);
    work_queue_destroy(sodb->q);
    free(db->vtable);
    free(sodb);
    free(db);
}

