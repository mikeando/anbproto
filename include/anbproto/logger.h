#ifndef ANBPROTO_LOGGER_H
#define ANBPROTO_LOGGER_H

typedef struct logger logger;

// Create a logger with its own mutex
void logger_init_root(logger** l, const char * name); 
// Create a logger that shares the mutex of another 
void logger_init_derived(logger**l, logger * parent, const char * name );
void logger_free_derived(logger*l);
void logger_free_root(logger*l);

void message( logger * l, const char * fmt, ...);

#endif
