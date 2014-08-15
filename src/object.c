#include "object.h"
#include "anbproto/structtypes.h"
#include <stdlib.h>
#include <string.h>


int anbp_object_create(anbp_object ** object, int id, int counter, const char * mesg) {
	anbp_object * obj = malloc(sizeof(anbp_object));
	smc_init_magic(anbp_object, obj);
	obj->id = id;
	obj->counter = counter;
    obj->mesg = strdup(mesg);
    *object = obj;
    return 0;
}
