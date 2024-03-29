#ifndef ANBPROTO_STRUCT_TYPES_H
#define ANBPROTO_STRUCT_TYPES_H

#include "simplemagic.h"

/* Randomly picked .. via </dev/random head -c 8 | xxd */
SMC_MAGIC(sqlite_thread_data     ,0x91b5f1f4c0fc2b50UL);
SMC_MAGIC(git_local_thread_data  ,0x8125a1db11c35be9UL);
SMC_MAGIC(git_remote_thread_data ,0x02fafa573f653e7aUL);
SMC_MAGIC(anbp_object            ,0xf9f06b9f261569d3UL);
SMC_MAGIC(sqlite3_worker_data    ,0xe948b1e3c9adcb30UL);
#endif
