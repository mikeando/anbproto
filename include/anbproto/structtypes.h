#ifndef ANBPROTO_STRUCT_TYPES_H
#define ANBPROTO_STRUCT_TYPES_H

#include <stdint.h>

/**
 * All my structs have a "magic" uint64_t at the start identifying the struct type
 * its not to be used for switching types or reflection etc, only for
 * sanity checking that a particular struct is of the specified type.
 *
 * It will probably be only enabled in debug builds some time soon...
 */

/* Randomly picked .. via </dev/random head -c 8 | xxd */
static uint64_t sqlite_thread_data_MAGIC    = 0x91b5f1f4c0fc2b50UL;
static uint64_t git_local_thread_data_MAGIC  = 0x8125a1db11c35be9UL;
static uint64_t git_remote_thread_data_MAGIC = 0x02fafa573f653e7aUL;
static uint64_t anbp_object_MAGIC            = 0xf9f06b9f261569d3UL;
static uint64_t worker_vtable_MAGIC          = 0xdede46705a77dfb9UL;
static uint64_t worker_MAGIC                 = 0x9d578c773cf81f41UL;
static uint64_t logger_MAGIC                 = 0xb0618b25b8917bf8UL;
static uint64_t work_queue_MAGIC             = 0x1c0504b859412982UL;
static uint64_t sqlite3_worker_data_MAGIC    = 0xe948b1e3c9adcb30UL;

#define check_struct_type(type,data) do { \
	type* vv = data; \
	check_struct_type_f(#type, #data, type##_MAGIC, (vv==NULL)?0:((type*)(data))->magic, vv, __LINE__,__FILE__); \
	} while(0)
void check_struct_type_f( const char * type, const char* expr, uint64_t expected_magic, uint64_t magic, void * p, int line, const char * file);

#endif
