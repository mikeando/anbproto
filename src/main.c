#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#include "anbproto/logger.h"
#include "anbproto/work_queue.h"



//TODO: Create a logging function (and maybe a logging thread?)

struct sqlite_thread_data {
	work_queue * queue;
	logger * root_logger;
};

typedef struct sqlite_thread_data sqlite_thread_data;

struct git_local_thread_data {
	work_queue * queue;
	logger * root_logger;
};

typedef struct git_local_thread_data git_local_thread_data;

struct git_remote_thread_data {
	work_queue * queue;
	logger * root_logger;
};

typedef struct git_remote_thread_data git_remote_thread_data;

typedef struct worker worker;

struct worker_vtable {
	void (*idle)(worker * );
	void (*process)(worker * ,work_queue_entry* entry);
};

struct worker {
	struct worker_vtable * vtable;
	work_queue * queue;
	const char * name;
	logger * root_logger;
	logger * logger;
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
}

void* sqlite_thread_fn(void * data) {
	sqlite_thread_data * d = data;
	struct worker_vtable vtable = {&idle, &process};
	struct worker w = { &vtable, d->queue, "SQLITE3", d->root_logger, NULL };
	run_worker(&w);
	return NULL;
}

void* git_local_thread_fn(void * data) {
	git_local_thread_data * d = data;
	struct worker_vtable vtable = {&idle, &process};
	struct worker w = { &vtable, d->queue, "GIT_L", d->root_logger, NULL };
	run_worker(&w);
	return NULL;
}

void* git_remote_thread_fn(void * data) {
	git_remote_thread_data * d = data;
	struct worker_vtable vtable = {&idle, &process};
	struct worker w = { &vtable, d->queue, "GIT_R", d->root_logger, NULL };
	run_worker(&w);
	return NULL;
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
	sqlite3_data.queue = sqlite3_queue;
	sqlite3_data.root_logger = root_logger;
	pthread_t sqlite3_thread;
	pthread_create(&sqlite3_thread, NULL, &sqlite_thread_fn, &sqlite3_data);
	message(root_logger,"Starting thread for local git operation...\n");

	work_queue * git_local_queue;
	work_queue_create(&git_local_queue);
	git_local_thread_data git_local_data;
	git_local_data.queue = git_local_queue;
	git_local_data.root_logger = root_logger;
	pthread_t git_local_thread;
	pthread_create(&git_local_thread, NULL, &git_local_thread_fn, &git_local_data);

	message(root_logger,"Starting thread for remote git operation...\n");
	work_queue * git_remote_queue;
	work_queue_create(&git_remote_queue);
	git_remote_thread_data git_remote_data;
	git_remote_data.queue = git_remote_queue;
	git_remote_data.root_logger = root_logger;
	pthread_t git_remote_thread;
	pthread_create(&git_remote_thread, NULL, &git_remote_thread_fn, &git_remote_data);

	work_queue * main_queue;
	work_queue_create(&main_queue);


	message(root_logger,"Telling remote git to refresh...\n");
	
	work_queue_add(git_remote_queue, work_queue_create_action("Refresh"));

	message(root_logger,"Fetching an object from sqlite3\n");
	
	//It needs to know where to stick the object once its loaded - thats the main queue.
	work_queue_add(sqlite3_queue, work_queue_create_actionX("Fetch Object", main_queue) );

	message(root_logger,"TODO: Updating object\n");
	// Wait for the response on the main queue.
	
	message(root_logger,"Saving object back to sqlite3\n");
	work_queue_add(sqlite3_queue, work_queue_create_actionX("Save Object", main_queue) );

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
