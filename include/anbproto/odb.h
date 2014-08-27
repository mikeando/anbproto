#ifndef ANBPROTO_ODB_H
#define ANBPROTO_ODB_H

#include <stdint.h>
#include <string.h>
#include "simplemagic.h"
#include "object.h"

/**********************************************
 * Object Database functions.
 **********************************************/
/**
 * The object database is stored in two pieces.  The fact that the code is
 * factored into two peices for performance should be quite transparent to the
 * user.
 *
 * The rebuildable sqlite metadata and index piece is used to store information
 * for quick retrieval and for fast searching. We consider this transient so
 * that conflicts due to git processes on the bulk data can be avoided, with
 * this structure kept out of memory and rebuilt on demand.
 *
 * The on disk object structure is simply flat text files stored into specified
 * disk locations. This format is used as it is very amenable to working with
 * the git sync/colaboration framework.
 *
 * The index and metadata are maintained by the scripting sub-system which is
 * notified whenever an object is added/modified/removed.
 *
 * The only automatic non-script index is the "type" property that all objects
 * are required to have.
 */

/**
 * @todo flesh out this struct better.
 */
struct index_key {
    int type;
    union {
        char *   key_s;
        int      key_i;
        uint64_t key_id;
    };
};
typedef struct index_key index_key;

struct request_range {
    const char * index;
    index_key min;
    index_key max;
};
typedef struct request_range request_range;

/**
 * (Abstract) Core of all object accessing functions.
 */

typedef struct odb odb;
typedef struct odb_vtable odb_vtable;
struct odb {
    SMC_ADD_MAGIC();
    odb_vtable * vtable;
    void * impl;
};

/**
 * Decalarations of request types
 */

typedef struct req_odb_get_ids req_odb_get_ids;
typedef struct req_odb_get_metas req_odb_get_metas;
typedef struct req_odb_get_objects req_odb_get_objects;
typedef struct req_odb_get_object req_odb_get_object;
typedef struct req_odb_put_object req_odb_put_object;

struct odb_vtable {
    int (*get_ids)(odb * self, req_odb_get_ids * ids );
    int (*get_metas)( odb * self, req_odb_get_metas * metas );
    int (*get_objects)( odb * self, req_odb_get_objects * objects );
    int (*get_object)( odb * self, req_odb_get_object * req );
    int (*put_object)( odb * self, req_odb_put_object * req );
};

/**
 * Array types
 */

typedef struct object_id_array object_id_array;
typedef struct object_meta_array object_meta_array;
typedef struct object_array object_array;


/**
 * Request a list of object ids from an index.
 *
 * Essentially runs a "SELECT id FROM index WHERE key>min AND key<max ORDER BY key"
 */


struct req_odb_get_ids {
    SMC_ADD_MAGIC();
    request_range range;
    void * userdata;
    /** Filled in when completed */
    object_id_array * result; 
    /** Called when completed */
    void (*done)(req_odb_get_ids* self);
};

static inline int odb_get_ids(odb * self, req_odb_get_ids * ids ) {
    return self->vtable->get_ids(self,ids);
}

/**
 * Request a list of object ids with meta data from an index
 */
struct req_odb_get_metas {
    SMC_ADD_MAGIC();
    request_range range;
    void * userdata;
    /** Filled in when completed */
    object_meta_array * result; 
    /** Called when completed */
    void (*done)(req_odb_get_metas* self);
};
static inline int odb_get_metas( odb * self, req_odb_get_metas * metas ) {
    return self->vtable->get_metas(self,metas);
}

/**
 * Request a list of objects from an index
 */
struct req_odb_get_objects {
    SMC_ADD_MAGIC();
    request_range range;
    void * userdata;
    /** Filled in when completed */
    object_array * result; 
    /** Called when completed */
    void (*done)(req_odb_get_objects * self);
};
static inline int odb_get_objects( odb * self, req_odb_get_objects * objects ) {
    return self->vtable->get_objects(self,objects);
}

/**
 * Request an object by id.
 */
struct req_odb_get_object {
    SMC_ADD_MAGIC();
    anbp_object_id id;
    void * userdata;
    /** Filled in when completed */
    anbp_object * result; 
    /** Called when completed */
    void (*done)(req_odb_get_object* self);
};
static inline int odb_get_object( odb * self, req_odb_get_object * req ) {
    return self->vtable->get_object(self,req);
}

/**
 * Save an object.
 */
struct req_odb_put_object {
    SMC_ADD_MAGIC();
    anbp_object * object;
    void * userdata;
    /** Called when completed */
    void (*done)(req_odb_put_object* self);
};

static inline int odb_put_object( odb * self, req_odb_put_object * req ) {
    return self->vtable->put_object(self,req);
}

#endif
