#ifndef ANBUTIL_STRUCT_TYPES_H
#define ANBUTIL_STRUCT_TYPES_H

#include "simplemagic.h"

/* Randomly picked .. via </dev/random head -c 8 | xxd */
SMC_MAGIC(worker_vtable          ,0xdede46705a77dfb9UL);
SMC_MAGIC(worker                 ,0x9d578c773cf81f41UL);
SMC_MAGIC(logger                 ,0xb0618b25b8917bf8UL);
SMC_MAGIC(work_queue             ,0x1c0504b859412982UL);
SMC_MAGIC(mesg_queue             ,0xb7598d4e740fa5feUL);
SMC_MAGIC(mesg_queue_entry       ,0x86fdc51ef81b698aUL);
SMC_MAGIC(odb                    ,0xe6492e74d21c224fUL);
SMC_MAGIC(simple_odb             ,0xb4ef73befb7b0966UL);
SMC_MAGIC(simple_odb_thread_data ,0xc537663a9eac80eeUL);
SMC_MAGIC(req_odb_get_object     ,0x819fa567d153977eUL);
SMC_MAGIC(req_odb_put_object     ,0xc04983607f87a533UL);
#endif

