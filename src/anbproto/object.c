#include "anbproto/object.h"
#include "anbproto/structtypes.h"
#include <stdlib.h>
#include <string.h>

/**
 * Note this creates a copy of the input id.
 */
int anbp_object_create(anbp_object ** object, anbp_object_id * id, int counter, const char * mesg) {
	anbp_object * obj = malloc(sizeof(anbp_object));
	smc_init_magic(anbp_object, obj);

    //Copy the id
    anbp_object_id * new_id = malloc(sizeof(anbp_object_id));
    char * id_str = malloc(id->length+1);
    memcpy(id_str, id->id, id->length);
    id_str[id->length]='\0';
    new_id->id = id_str;
    new_id->length = id->length;
    

    obj->id = new_id;
    obj->counter = counter;
    obj->mesg = strdup(mesg);
    *object = obj;
    return 0;
}

void anbp_object_free(anbp_object * object) {
    free((void*)object->id->id);
    free(object->id);
    free((void*)object->mesg);
    free(object);
}
