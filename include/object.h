#ifndef ANBP_OBJECT_H
#define ANBP_OBJECT_H

#include "simplemagic.h"
typedef struct anbp_object anbp_object;

// Dummy object 
struct anbp_object {
	SMC_ADD_MAGIC();
    int id;
	int counter;
    const char * mesg;
};

int anbp_object_create(anbp_object ** object, int id, int counter, const char * mesg);

#endif

