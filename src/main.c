#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

struct logger {
	pthread_mutex_t * lock;
	const char * prefix;
};

typedef struct logger logger;

// Create a logger with its own mutex
void logger_init_root(logger** l, const char * name) {
	logger * ll = malloc(sizeof(logger));
	ll->lock = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(ll->lock, NULL);
	ll->prefix = name;
	*l = ll;
}

// Create a logger that shares the mutex of another 
void logger_init_derived(logger**l, logger * parent, const char * name ) {
	logger * ll = malloc(sizeof(logger));
	ll->lock = parent->lock;
	ll->prefix = name;
	*l = ll;
}

void message( logger * l, const char * fmt, ...) {

	pthread_mutex_lock(l->lock);

	char * mesg=NULL;

	va_list ap;
	va_start(ap,fmt);
	int ok = vasprintf(&mesg,fmt,ap);
	va_end(ap);

	if(ok<0) {
		printf("%s : Formatting message failed\n", l->prefix);
		pthread_mutex_unlock(l->lock);
		return;
	}

	printf("%s : %s", l->prefix, mesg);
	pthread_mutex_unlock(l->lock);
}

struct work_queue {
	pthread_mutex_t lock;
	int poisoned;
};

typedef struct work_queue work_queue;

void work_queue_create(work_queue **q) {
	work_queue * retval = malloc(sizeof(work_queue));
	memset(retval, 0, sizeof(work_queue));
	retval->poisoned = 0;
	pthread_mutex_init(&retval->lock, NULL);
	*q = retval;
};

void work_queue_destroy(work_queue *q) {
	pthread_mutex_destroy(&q->lock);
	free(q);
};

void work_queue_poison(work_queue *q) {
	pthread_mutex_lock(&q->lock);
	q->poisoned = 1;
	pthread_mutex_unlock(&q->lock);
};

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

void* sqlite_thread_fn(void * data) {
	sqlite_thread_data * d = data;
	work_queue * q = d->queue;
	logger * logx;
	logger_init_derived(&logx, d->root_logger, "SQLITE3");
	message(logx," --- In sqlite thread\n");
	while(1) {
		if(pthread_mutex_trylock(&q->lock)==0) {
			if(q->poisoned==1) {
				pthread_mutex_unlock(&q->lock);
				break;
			}
			message(logx,"Trying to consume stuff off my work queue\n");
			pthread_mutex_unlock(&q->lock);
		} else {
			message(logx,"Spinning...\n");
		}
		usleep(10);
	}

	message(logx," --- sqlite thread exiting\n");
	return NULL;
}

void* git_local_thread_fn(void * data) {
	git_local_thread_data * d = data;
	work_queue * q = d->queue;
	logger * logx;
	logger_init_derived(&logx, d->root_logger, "GIT_L");
	message(logx," --- In git local thread\n");
	while(1) {
		if(pthread_mutex_trylock(&q->lock)==0) {
			if(q->poisoned==1) {
				pthread_mutex_unlock(&q->lock);
				break;
			}
			message(logx,"Trying to consume stuff off my work queue\n");
			pthread_mutex_unlock(&q->lock);
		} else {
			message(logx,"Spinning...\n");
		}
		usleep(10);
	}

	message(logx," --- git local thread exiting\n");
	return NULL;
}

void* git_remote_thread_fn(void * data) {
	git_remote_thread_data * d = data;
	work_queue * q = d->queue;
	logger * logx;
	logger_init_derived(&logx, d->root_logger, "GIT_R");
	message(logx," --- In git remote thread\n");
	while(1) {
		if(pthread_mutex_trylock(&q->lock)==0) {
			if(q->poisoned==1) {
				pthread_mutex_unlock(&q->lock);
				break;
			}
			message(logx,"Trying to consume stuff off my work queue\n");
			pthread_mutex_unlock(&q->lock);
		} else {
			message(logx,"Spinning...\n");
		}
		usleep(10);
	}

	message(logx," --- git remote thread exiting\n");
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


	message(root_logger,"Telling remote git to refresh...\n");

	message(root_logger,"Fetching an object from sqlite3\n");
	message(root_logger,"Updating object\n");
	message(root_logger,"Saving object back to sqlite3\n");

	message(root_logger,"Getting local git changes\n");
	message(root_logger,"Telling local git to commit changes\n");

	message(root_logger,"Telling remote git to push\n");

	message(root_logger,"Napping...\n");
	usleep(100);

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

	message(root_logger,"Done\n");

	return 0;

error_exit:
	if (d!=NULL) {
		closedir(d);
	}

	return 1;
}
