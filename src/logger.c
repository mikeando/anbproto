#include "anbproto/logger.h"
#include <stdlib.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>

#include "anbproto/structtypes.h"
#include "simplemagic.h"

struct logger {
	SMC_ADD_MAGIC();
	pthread_mutex_t * lock;
	const char * prefix;
};

// Create a logger with its own mutex
void logger_init_root(logger** l, const char * name) {
	logger * ll = malloc(sizeof(logger));
	smc_init_magic(logger, ll);
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

	pthread_mutex_lock(l->lock);
	printf("%s : %s", l->prefix, mesg);
	pthread_mutex_unlock(l->lock);
	free(mesg);
}

void logger_free_derived(logger*l) {
	free(l);
}

void logger_free_root(logger*l) {
	pthread_mutex_destroy(l->lock);
	free(l->lock);
	free(l);
}

