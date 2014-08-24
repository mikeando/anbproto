#ifndef ANBPROTO_LOGGER_H
#define ANBPROTO_LOGGER_H

typedef struct logger logger;

/**
 * Create a logger with its own mutex.
 *
 * Needs to be destroyed with the logger_free_root function.
 *
 * Must not be destroyed until all loggers derived from this logger 
 * have been destroyed.
 *
 * @param l a pointer to the logger that is created
 * @param name the prefix used to identify output from this logger.
 */
void logger_init_root(logger** l, const char * name); 


/**
 * Create a logger that shares the mutex of another.
 *
 * This means the two loggers can be used on different threads without log messages 
 * becoming intermixed.
 *
 * Needs to be destroyed using logger_free_derived.
 *
 * Must be destroyed before the root logger is destroyed.
 *
 * @param l a pointer to the logger that is created.
 * @parent a logger who's mutex we will share.
 * @param name the prefix used to identify output from this logger.
 */
void logger_init_derived(logger**l, logger * parent, const char * name );


/**
 * Free a logger created with logger_init_derived.
 *
 * Must be called before the root logger is freed.
 */
void logger_free_derived(logger*l);


/**
 * Free a logger created with logger_init_root.
 *
 * Must be called after all derived loggers are freed.
 */
void logger_free_root(logger*l);


/**
 * Prints a message using the given logger and its prefix. 
 *
 * The function takes a format message and additional arguments in the same
 * manner as printf.
 *
 * @param l the logger to print the output.
 * @param fmt the format string in printf style to use for the arguments.
 * @param ... additional arguments in printf style.
 */
void message( logger * l, const char * fmt, ...);

#endif
