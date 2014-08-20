#ifndef ANBP_OBJECT_H
#define ANBP_OBJECT_H

#include "simplemagic.h"
#include <string.h>

typedef struct anbp_object_id anbp_object_id;
typedef struct anbp_object anbp_object;

struct anbp_object_id {
    size_t length;
    const char * id;
};

// Dummy object 
struct anbp_object {
	SMC_ADD_MAGIC();
    anbp_object_id * id;
	int counter;
    const char * mesg;
};

int anbp_object_create(anbp_object ** object, anbp_object_id *id, int counter, const char * mesg);

#endif

