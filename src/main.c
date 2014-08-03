#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#include <sqlite3.h>
#include <assert.h>

#include "anbproto/logger.h"
#include "anbproto/work_queue.h"
#include "anbproto/structtypes.h"

struct sqlite_thread_data {
	uint64_t magic;
	work_queue * queue;
	logger * root_logger;
	const char * filename;
};

typedef struct sqlite_thread_data sqlite_thread_data;

struct git_local_thread_data {
	uint64_t magic;
	work_queue * queue;
	logger * root_logger;
};

typedef struct git_local_thread_data git_local_thread_data;

struct git_remote_thread_data {
	uint64_t magic;
	work_queue * queue;
	logger * root_logger;
};

typedef struct git_remote_thread_data git_remote_thread_data;

struct worker_vtable {
	uint64_t magic;
	void (*idle)(worker * );
	void (*process)(worker * ,work_queue_entry* entry);
};

struct worker {
	uint64_t magic;
	struct worker_vtable * vtable;
	work_queue * queue;
	const char * name;
	logger * root_logger;
	logger * logger;
	void * user_data;
};

void run_worker( struct worker * w ) {
	work_queue * q = w->queue;
	logger * logx;
	logger_init_derived(&logx, w->root_logger, w->name);
	w->logger = logx;
	message(logx," --- Started worker thread\n");
	while(1) {
		if(pthread_mutex_trylock(&q->lock)==0) {
			if(q->poisoned==1) {
				pthread_mutex_unlock(&q->lock);
				break;
			}
			message(logx,"Trying to consume stuff off my work queue\n");
			if(q->read_cursor < q->write_cursor) {
				message(logx, "I have an entry\n");
				work_queue_entry * entry = q->entries[q->read_cursor];
				q->read_cursor++;
				pthread_mutex_unlock(&q->lock);

				w->vtable->process(w,entry);
				free(entry);
			} else {
				message(logx,"My queue is empty...\n");
				pthread_mutex_unlock(&q->lock);
				w->vtable->idle(w);
				usleep(10);
			}
		} else {
			w->vtable->idle(w);
			usleep(10);
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
	uint64_t magic;
	sqlite3 * db;
	sqlite3_stmt * select_stmt;
};
typedef struct sqlite3_worker_data sqlite3_worker_data;

// Dummy object 
// TODO: Pull this from the SQLITE...
struct anbp_object {
	uint64_t magic;
	int counter;
};

typedef struct anbp_object anbp_object;

void* sqlite_thread_fn(void * data) {
	check_struct_type(sqlite_thread_data, data);
	sqlite_thread_data * d = data;

	struct sqlite3_worker_data wd;
	wd.magic = sqlite3_worker_data_MAGIC;

	sqlite3_open(d->filename, &wd.db);

	const char * sql = "SELECT * FROM monkey";
	int sql_len = strlen(sql);
	sqlite3_prepare_v2(wd.db,sql,sql_len, &wd.select_stmt, NULL);
	
	struct worker_vtable vtable = { worker_vtable_MAGIC, &idle, &process};
	struct worker w = { worker_MAGIC, &vtable, d->queue, "SQLITE3", d->root_logger, NULL, &wd };
	run_worker(&w);
	sqlite3_finalize(wd.select_stmt);
	sqlite3_close(wd.db);
	return NULL;
}

void* git_local_thread_fn(void * data) {
	check_struct_type(git_local_thread_data, data);
	git_local_thread_data * d = data;
	struct worker_vtable vtable = {worker_vtable_MAGIC, &idle, &process};
	struct worker w = { worker_MAGIC, &vtable, d->queue, "GIT_L", d->root_logger, NULL, NULL };
	run_worker(&w);
	return NULL;
}

void* git_remote_thread_fn(void * data) {
	check_struct_type(git_remote_thread_data, data);
	git_remote_thread_data * d = data;
	struct worker_vtable vtable = {worker_vtable_MAGIC, &idle, &process};
	struct worker w = { worker_MAGIC, &vtable, d->queue, "GIT_R", d->root_logger, NULL, NULL };
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

void db_fetch_process(work_queue_entry * self, worker * w) {
	message(w->logger, "Loading object... (IMPLEMENT ME)\n");

	check_struct_type(sqlite3_worker_data, w->user_data);
	struct sqlite3_worker_data * wd = (struct sqlite3_worker_data*) w->user_data;

	int status;
	sqlite3_reset(wd->select_stmt);
	sqlite3_clear_bindings(wd->select_stmt);
	message(w->logger,"SQLITE: getting rows\n");	
	while( (status = sqlite3_step(wd->select_stmt)) == SQLITE_ROW ) {
		message(w->logger,"SQLITE: Got me a row\n");	
	}

	if(status != SQLITE_DONE) {
		message(w->logger, "Error (%d) : %s\n", status, sqlite3_errmsg(wd->db));
		//TODO: Let caller know its failed...
		return;
	}

	anbp_object * obj = malloc(sizeof(anbp_object));
	obj->magic = anbp_object_MAGIC;
	obj->counter = 1;

	//TODO: Do something with obj.
	check_struct_type(work_queue, self->user_data);
	work_queue * q = (work_queue*)self->user_data;
	work_queue_add(q, work_queue_create_action("FETCHED", -1, &do_fetched, NULL));
}

void db_save_process(work_queue_entry * entry, worker * w) {
	message(w->logger, "Saving object... (IMPLEMENT ME)\n");
	//TODO: Get the object from the entry and save it...
	anbp_object * obj = malloc(sizeof(anbp_object));
	obj->magic = anbp_object_MAGIC;

	//TODO: Do something with obj.
	check_struct_type(work_queue, entry->user_data);
	work_queue * q = (work_queue*)entry->user_data;
	work_queue_add(q, work_queue_create_action("SAVED", -1, &do_saved, NULL));
}


int main() {

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
	sqlite3_data.magic = sqlite_thread_data_MAGIC;
	sqlite3_data.queue = sqlite3_queue;
	sqlite3_data.root_logger = root_logger;
	sqlite3_data.filename = "./dummy.sqlite3";
	pthread_t sqlite3_thread;
	pthread_create(&sqlite3_thread, NULL, &sqlite_thread_fn, &sqlite3_data);
	message(root_logger,"Starting thread for local git operation...\n");

	work_queue * git_local_queue;
	work_queue_create(&git_local_queue);
	git_local_thread_data git_local_data;
	git_local_data.magic = git_local_thread_data_MAGIC;
	git_local_data.queue = git_local_queue;
	git_local_data.root_logger = root_logger;
	pthread_t git_local_thread;
	pthread_create(&git_local_thread, NULL, &git_local_thread_fn, &git_local_data);

	message(root_logger,"Starting thread for remote git operation...\n");
	work_queue * git_remote_queue;
	work_queue_create(&git_remote_queue);
	git_remote_thread_data git_remote_data;
	git_remote_data.magic = git_remote_thread_data_MAGIC;
	git_remote_data.queue = git_remote_queue;
	git_remote_data.root_logger = root_logger;
	pthread_t git_remote_thread;
	pthread_create(&git_remote_thread, NULL, &git_remote_thread_fn, &git_remote_data);

	work_queue * main_queue;
	work_queue_create(&main_queue);


	message(root_logger,"Telling remote git to refresh...\n");
	
	work_queue_add(git_remote_queue, work_queue_create_action("Refresh", -1, do_refresh, NULL));

	message(root_logger,"Fetching an object from sqlite3\n");
	
	//It needs to know where to stick the object once its loaded - thats the main queue.
	work_queue_add(sqlite3_queue, work_queue_create_action("Fetch Object", ANBP_DB_LOAD_OBJECT, db_fetch_process,  main_queue) );

	message(root_logger,"TODO: Updating object\n");
	// TODO: Wait for the response on the main queue.
	
	message(root_logger,"Saving object back to sqlite3\n");
	work_queue_add(sqlite3_queue, work_queue_create_action("Save Object", ANBP_DB_SAVE_OBJECT, db_save_process, main_queue) );

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
	work_queue_destroy( main_queue );

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
