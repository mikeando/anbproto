#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#include <sqlite3.h>
#include <assert.h>

#include "anbutil/logger.h"
#include "anbutil/work_queue.h"
#include "anbutil/structtypes.h"
#include "anbproto/structtypes.h"

//TODO: remove sqlite_thread and git threads. 
//      thats all going to hide behind the simple_odb backend.

struct sqlite_thread_data {
	SMC_ADD_MAGIC();
	work_queue * queue;
	logger * root_logger;
	const char * filename;
};

typedef struct sqlite_thread_data sqlite_thread_data;

struct git_local_thread_data {
	SMC_ADD_MAGIC();
	work_queue * queue;
	logger * root_logger;
};

typedef struct git_local_thread_data git_local_thread_data;

struct git_remote_thread_data {
	SMC_ADD_MAGIC();
	work_queue * queue;
	logger * root_logger;
};

typedef struct git_remote_thread_data git_remote_thread_data;

//TODO: Move me into the work_queue file?
//      And correspondingly trim down the work_queue visible interface.
//      Distinguish ANBPROTO_QUEUE_EMPTY and ANBPROTO_QUEUE_BUSY vtable entries
void run_worker( struct worker * w ) {
	work_queue * q = w->queue;
	logger * logx;
	logger_init_derived(&logx, w->root_logger, w->name);
	w->logger = logx;
	message(logx," --- Started worker thread\n");
	work_queue_entry * entry;
	int ok;
	while((ok=work_queue_take(q,&entry))!=ANBPROTO_QUEUE_DONE) {

		message(logx,"Trying to consume stuff off my work queue\n");
		if(ok==ANBPROTO_QUEUE_OK) {
			message(logx, "I have an entry\n");
			w->vtable->process(w,entry);
			work_queue_entry_destroy(entry);
		} else if( ok==ANBPROTO_QUEUE_EMPTY ){
			message(logx,"My queue is empty...\n");
			w->vtable->idle(w);
			usleep(100000);
		} else if (ok==ANBPROTO_QUEUE_BUSY ) {
			message(logx,"My queue is empty...\n");
			w->vtable->idle(w);
			usleep(100000);
		} else {
			assert(!"Invalid state returned by work_queue_take");
		}
	}

	message(logx," --- worker thread exiting\n");
	logger_free_derived(logx);
}

void idle(worker * w) {
	message(w->logger, "idling...\n");
}

void process( worker * w, work_queue_entry * entry ) {
	message(w->logger,"My entry is : %s\n", entry->name);
	assert(entry->vtable!=NULL);
	entry->vtable->process(entry, w);
}

struct sqlite3_worker_data {
	SMC_ADD_MAGIC();
	sqlite3 * db;
	sqlite3_stmt * select_stmt;
	sqlite3_stmt * update_stmt;
};
typedef struct sqlite3_worker_data sqlite3_worker_data;

#include "anbproto/object.h"


void* sqlite_thread_fn(void * data) {
	smc_check_type(sqlite_thread_data, data);
	sqlite_thread_data * d = data;

	sqlite3_worker_data wd;
	smc_init_magic(sqlite3_worker_data, &wd);

	sqlite3_open(d->filename, &wd.db);

    {
        const char * sql = "SELECT id, counter, mesg FROM monkey WHERE id=?";
        int sql_len = strlen(sql);
        sqlite3_prepare_v2(wd.db,sql,sql_len, &wd.select_stmt, NULL);
    }
    {
        const char * sql = "UPDATE monkey SET counter=?, mesg=? WHERE id=?";
        int sql_len = strlen(sql);
        sqlite3_prepare_v2(wd.db,sql,sql_len, &wd.update_stmt, NULL);
    }
	
	struct worker_vtable vtable = { smc__magic_worker_vtable, &idle, &process};
	struct worker w = { smc__magic_worker, &vtable, d->queue, "SQLITE3", d->root_logger, NULL, &wd };
	run_worker(&w);
	sqlite3_finalize(wd.select_stmt);
	sqlite3_close(wd.db);
	return NULL;
}

void* git_local_thread_fn(void * data) {
	smc_check_type(git_local_thread_data, data);
	git_local_thread_data * d = data;
	struct worker_vtable vtable = {smc__magic_worker_vtable, &idle, &process};
	struct worker w = { smc__magic_worker, &vtable, d->queue, "GIT_L", d->root_logger, NULL, NULL };
	run_worker(&w);
	return NULL;
}

void* git_remote_thread_fn(void * data) {
	smc_check_type(git_remote_thread_data, data);
	git_remote_thread_data * d = data;
	struct worker_vtable vtable = {smc__magic_worker_vtable, &idle, &process};
	struct worker w = { smc__magic_worker, &vtable, d->queue, "GIT_R", d->root_logger, NULL, NULL };
	run_worker(&w);
	return NULL;
}
void do_fetched(work_queue_entry* self, worker* w) {
	message(w->logger, "in do_refresh...\n");
	//TODO: Implement me
}

void do_refresh(work_queue_entry* self, worker* w) {
	message(w->logger, "in do_refresh...\n");
	//TODO: Implement me
}

void do_saved(work_queue_entry* self, worker* w) {
	message(w->logger, "in do_saved...\n");
	//TODO: Implement me
}

//TODO: Move me
typedef struct db_action_fetch db_action_fetch;
struct db_action_fetch {
    SMC_ADD_MAGIC();
    const char * target_id;
    mesg_queue * q;
};

SMC_MAGIC(db_action_fetch, 0x12344321UL);

//TODO: Move me
typedef struct db_action_save db_action_save;
struct db_action_save {
    SMC_ADD_MAGIC();
    anbp_object * object;
    mesg_queue * q;
};

SMC_MAGIC(db_action_save, 0x43211234UL);

#include "anbproto/odb.h"
#include "script.h"


void db_fetch_process(work_queue_entry * self, worker * w) {

	smc_check_type(sqlite3_worker_data, w->user_data);
	struct sqlite3_worker_data * wd = (struct sqlite3_worker_data*) w->user_data;

    smc_check_type(db_action_fetch, self->user_data);
    db_action_fetch* action = (db_action_fetch*)self->user_data;

	int status;
	sqlite3_reset(wd->select_stmt);
	sqlite3_clear_bindings(wd->select_stmt);
    sqlite3_bind_text(wd->select_stmt, 1, action->target_id, strlen(action->target_id), SQLITE_TRANSIENT);
    
	message(w->logger,"SQLITE: getting rows\n");	
	status = sqlite3_step(wd->select_stmt);
    if(status!=SQLITE_ROW) {
        // It's some kind of error .. we've either got no result
        // or we've got a real error!
        if(status == SQLITE_DONE) {
            message(w->logger, "Error (%d) : No result from query\n");
            //TODO: Let caller know its failed...
            return;
        }

		message(w->logger, "Error (%d) : %s\n", status, sqlite3_errmsg(wd->db));
		//TODO: Let caller know its failed...
		return;
    }


    message(w->logger,"SQLITE: Got me a row\n");	
    message(w->logger, "row : id=%s counter=%d mesg=%s\n", 
            sqlite3_column_text(wd->select_stmt, 0),
            sqlite3_column_int(wd->select_stmt, 1),
            sqlite3_column_text(wd->select_stmt, 2)
           );

   
   anbp_object_id id;
   id.length = sqlite3_column_bytes(wd->select_stmt,0);
   id.id = (const char*)sqlite3_column_text(wd->select_stmt,0);

	anbp_object * obj = NULL;
    anbp_object_create(
            &obj,
            &id,
            sqlite3_column_int(wd->select_stmt, 1),
            (const char*)sqlite3_column_text(wd->select_stmt, 2)
            );

	mesg_queue * q = action->q;

    //TODO: Should use a better type than this.
    mesg_queue_entry * e = mesg_queue_entry_create(obj);
	mesg_queue_add(q, e);
}

void db_save_process(work_queue_entry * entry, worker * w) {
	message(w->logger, "Saving object...\n");

	smc_check_type(sqlite3_worker_data, w->user_data);
	struct sqlite3_worker_data * wd = (struct sqlite3_worker_data*) w->user_data;

	smc_check_type(db_action_save, entry->user_data);
    db_action_save * action = (db_action_save*)entry->user_data;
	mesg_queue * q = action->q;
    
	anbp_object * obj = action->object;

    message(w->logger,"Should be saving object id=%s, counter=%d, mesg=%s\n", obj->id->id, obj->counter, obj->mesg);

	int status;
	sqlite3_reset(wd->update_stmt);
	sqlite3_clear_bindings(wd->update_stmt);
    sqlite3_bind_int(wd->update_stmt, 1, obj->counter);
    sqlite3_bind_text(wd->update_stmt, 2, obj->mesg, strlen(obj->mesg), SQLITE_TRANSIENT);
    sqlite3_bind_text(wd->update_stmt, 3, obj->id->id, obj->id->length, SQLITE_TRANSIENT);
	status = sqlite3_step(wd->update_stmt);

    if(status!=SQLITE_DONE) {
		message(w->logger, "Error saving to db (%d) : %s\n", status, sqlite3_errmsg(wd->db));
    }

    mesg_queue_entry * e = mesg_queue_entry_create(obj);
	mesg_queue_add(q, e);
}

void got_object(req_odb_get_object* req) {
    printf("Get object request completed: req = %p\n",req);
    //TODO: Assumes 0 terminated.
    printf("   req.object_id = %s\n" , req->id.id);
    printf("   req.result = %p\n", req->result);
    smc_check_type(mesg_queue,req->userdata);
    mesg_queue * q = req->userdata;
    mesg_queue_entry * e = mesg_queue_entry_create(req->result);
	mesg_queue_add(q, e);
}

void put_object(req_odb_put_object* req) {
    printf("Put object request copmleted: req = %p\n",req);
    anbp_object_free(req->object);
    //TODO: Assumes 0 terminated.
    /*
    smc_check_type(mesg_queue,req->userdata);
    mesg_queue * q = req->userdata;
    mesg_queue_entry * e = mesg_queue_entry_create(req->result);
	mesg_queue_add(q, e);
    */
}

#include "anbproto/simple_odb.h"



int main2() {
	logger * root_logger;
	logger_init_root(&root_logger, "MAIN");

    odb * db = simple_odb_init(root_logger);

	mesg_queue * main_queue;
	mesg_queue_create(&main_queue);

    req_odb_get_object req;
    smc_init_magic(req_odb_get_object, &req);
    req.id.id = "some_id";
    req.id.length = strlen(req.id.id);
    req.userdata = main_queue;
    req.done = &got_object;

    odb_get_object(db, &req);

    //Wait to get the object back on the main mesg queue
    anbp_object * obj = NULL;
    {
        mesg_queue_entry * e;
        {
            int ct=0;
            while(mesg_queue_take(main_queue, &e)!=ANBPROTO_QUEUE_OK) {
                ++ct;
                if(ct>1000*100) {
                    message(root_logger,"Didn't get my object :(\n");
                    exit(1);
                }

                usleep(100);
            }
        }

        smc_check_type(anbp_object, e->user_data);
        obj = e->user_data;
        message(root_logger,"Got object: id=%s, counter=%d, mesg=%s\n", obj->id->id, obj->counter, obj->mesg);
        obj->counter++;
        mesg_queue_entry_destroy(e);
        //TODO: Presumably we need to free the obj too.
    }

    req_odb_put_object req_put;
    smc_init_magic(req_odb_put_object, &req_put);
    req_put.object = obj;
    req_put.done = &put_object;

    odb_put_object(db, &req_put);


    simple_odb_destroy(db);
    mesg_queue_destroy(main_queue);
    logger_free_root(root_logger);
    return 0;
}


int main1() {

	logger * root_logger;
	logger_init_root(&root_logger, "MAIN");

	DIR * d = NULL;

	message(root_logger, "Starting ANB Prototype...\n");

	message(root_logger, "Finding application root\n");

	//TODO: Use tilde expansion here, or HOME env variable?
	const char * home = getenv("HOME");
	char app_root_path[1024];
	snprintf(app_root_path,1024,"%s/anbproto", home);
	d = opendir(app_root_path);
	if(d==NULL) {
		message(root_logger, "Error opening root: %s\n",app_root_path);
		goto error_exit;
	}
	message(root_logger," -- opened %s\n", app_root_path);


	message(root_logger,"Loading git configuration...\n");
	message(root_logger," -- NYI\n");

	message(root_logger,"Loading sqlite3 configuration...\n");
	message(root_logger," -- NYI\n");

	closedir(d);
	d=NULL;

	message(root_logger,"Starting sqlite3 thread...\n");
	work_queue * sqlite3_queue;
	work_queue_create(&sqlite3_queue);
	sqlite_thread_data sqlite3_data;
	smc_init_magic(sqlite_thread_data, &sqlite3_data);
	sqlite3_data.queue = sqlite3_queue;
	sqlite3_data.root_logger = root_logger;
	sqlite3_data.filename = "./dummy.sqlite3";
	pthread_t sqlite3_thread;
	pthread_create(&sqlite3_thread, NULL, &sqlite_thread_fn, &sqlite3_data);
	message(root_logger,"Starting thread for local git operation...\n");

	work_queue * git_local_queue;
	work_queue_create(&git_local_queue);
	git_local_thread_data git_local_data;
	smc_init_magic(git_local_thread_data, &git_local_data);
	git_local_data.queue = git_local_queue;
	git_local_data.root_logger = root_logger;
	pthread_t git_local_thread;
	pthread_create(&git_local_thread, NULL, &git_local_thread_fn, &git_local_data);

	message(root_logger,"Starting thread for remote git operation...\n");
	work_queue * git_remote_queue;
	work_queue_create(&git_remote_queue);
	git_remote_thread_data git_remote_data;
	smc_init_magic(git_remote_thread_data, &git_remote_data);
	git_remote_data.queue = git_remote_queue;
	git_remote_data.root_logger = root_logger;
	pthread_t git_remote_thread;
	pthread_create(&git_remote_thread, NULL, &git_remote_thread_fn, &git_remote_data);

	mesg_queue * main_queue;
	mesg_queue_create(&main_queue);


	message(root_logger,"Telling remote git to refresh...\n");
	
	work_queue_add(git_remote_queue, work_queue_create_action("Refresh", -1, do_refresh, NULL));

	message(root_logger,"Fetching an object from sqlite3\n");
	
	//It needs to know where to stick the object once its loaded - thats the main queue.
    db_action_fetch fetch;
    smc_init_magic(db_action_fetch, &fetch);
    fetch.target_id = "object_a";
    fetch.q = main_queue;

	work_queue_add(sqlite3_queue, work_queue_create_action("Fetch Object", ANBP_DB_LOAD_OBJECT, db_fetch_process,  &fetch) );

    anbp_object * obj = NULL;
    {
        mesg_queue_entry * e;
        {
            int ct=0;
            while(mesg_queue_take(main_queue, &e)!=ANBPROTO_QUEUE_OK) {
                ++ct;
                if(ct>1000*100) {
                    message(root_logger,"Didn't get my object :(\n");
                    exit(1);
                }

                usleep(100);
            }
        }

        smc_check_type(anbp_object, e->user_data);
        obj = e->user_data;
        message(root_logger,"Got object: id=%s, counter=%d, mesg=%s\n", obj->id->id, obj->counter, obj->mesg);
        obj->counter++;
        mesg_queue_entry_destroy(e);
    }
	
	message(root_logger,"Saving object back to sqlite3\n");

    db_action_save save;
    smc_init_magic(db_action_save, &save);
    save.object = obj;
    save.q = main_queue;

	work_queue_add(sqlite3_queue, work_queue_create_action("Save Object", ANBP_DB_SAVE_OBJECT, db_save_process, &save) );

    {
        mesg_queue_entry * e;
        {
            int ct=0;
            while(mesg_queue_take(main_queue, &e)!=ANBPROTO_QUEUE_OK) {
                ++ct;
                if(ct>1000*100) {
                    message(root_logger,"Didn't get my object :(\n");
                    exit(1);
                }

                usleep(100);
            }
        }

        message(root_logger,"TODO: object %p saved...\n", e);
        smc_check_type(anbp_object, e->user_data);
        anbp_object_free((anbp_object*)e->user_data);
        mesg_queue_entry_destroy(e);
    }

	message(root_logger,"Getting local git changes\n");
	message(root_logger,"Telling local git to commit changes\n");

	message(root_logger,"Telling remote git to push\n");

	message(root_logger,"Napping...\n");
	usleep(1000);

	message(root_logger,"Poisoning the worker threads\n");

	work_queue_poison(sqlite3_queue);
	work_queue_poison(git_local_queue);
	work_queue_poison(git_remote_queue);

	message(root_logger,"Waiting for worker threads to exit\n");
	pthread_join(sqlite3_thread, NULL);
	pthread_join(git_local_thread, NULL);
	pthread_join(git_remote_thread, NULL);

	message(root_logger,"Destroying work queues\n");
	work_queue_destroy( sqlite3_queue );
	work_queue_destroy( git_local_queue );
	work_queue_destroy( git_remote_queue );
	mesg_queue_destroy( main_queue );

	message(root_logger,"Done\n");

	logger_free_root(root_logger);



	return 0;

error_exit:
	if (d!=NULL) {
		closedir(d);
	}

	logger_free_root(root_logger);

	return 1;
}

int main() {
    main1();
    main2();
}
