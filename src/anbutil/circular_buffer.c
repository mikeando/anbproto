#include "anbutil/circular_buffer.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

/**
 * We allocate a buffer one larger than the requested size since the fullest
 * a circular buffer can be is
 *
 * O O O O X O O O O O O O
 *         W R
 *
 * With the write head one unit behind the read head.
 *
 * This means we never run into this case 
 *
 * O O O O O O O O O O O O
 *         R
 *         W
 *
 * which is indistinguishable from an
 * empty buffer
 *
 * X X X X X X X X X X X X
 *         R
 *         W
 */

void circular_buffer_create(
        circular_buffer ** circ,
        uint32_t initial_capacity ) {
    circular_buffer * retval = malloc(sizeof(circular_buffer));
	retval->data = malloc(sizeof(void*)*(initial_capacity+1));
	retval->buffer_length = initial_capacity+1;
	retval->read_cursor = 0;  // Where the next read will read from
	retval->write_cursor = 0; // Where the next add will write to
  *circ = retval;
}


uint32_t circular_buffer_size(
        circular_buffer * circ
        ) {
    if(circ->read_cursor <= circ->write_cursor) {
        return circ->write_cursor - circ->read_cursor;
    }
    return circ->buffer_length - circ->read_cursor + circ->write_cursor;
}

int circular_buffer_get(
        circular_buffer * circ,
        void ** p
        ) {

    if(circular_buffer_size(circ)<=0) {
        return ECIRC_EMPTY;
    }

    *p = circ->data[circ->read_cursor];
    circ->read_cursor = (circ->read_cursor + 1) % circ->buffer_length;

    return ECIRC_OK;
}

int circular_buffer_put(
        circular_buffer * circ,
        void * p
        ) {
    if(circular_buffer_size(circ)>=circ->buffer_length-1) {
        return ECIRC_FULL;
    }

    circ->data[circ->write_cursor]=p;
    circ->write_cursor = (circ->write_cursor + 1) % circ->buffer_length;
    return ECIRC_OK;
}

void * circular_buffer_peek(
        circular_buffer * circ 
        ) {
    if(circular_buffer_size(circ)<=0) {
        assert(!"circular_buffer_peek called on empty buffer");
        return NULL;
    }

    return circ->data[circ->read_cursor];
}

void circular_buffer_expand(
        circular_buffer * circ,
        uint32_t delta 
        ) {
    //TODO: Could be using realloc here to avoid copies in some cases.
    int new_size = circ->buffer_length + delta;
    void** new_data = malloc(sizeof(void*)*new_size);
    uint32_t length = circular_buffer_size(circ);
    if(length==0) {
        // Buffer looks like this
        // 
        // X X X X X X X X X X X X X X
        //           R         
        //           W
    } else if (circ->read_cursor < circ->write_cursor) {
        // Buffer looks like this
        // 
        // X X X X X O O O O O X X X X
        //           R         W
        memcpy(new_data, circ->data+circ->read_cursor, sizeof(void*)*length);
    } else {
        // Buffer looks like this
        // 
        // O O O O O X X X X X O O O O
        //           W         R
        uint32_t after_read_cursor = circ->buffer_length - circ->read_cursor;
        memcpy(new_data, circ->data+circ->read_cursor, sizeof(void*)*after_read_cursor);
        memcpy(new_data+after_read_cursor, circ->data, sizeof(void*)*circ->write_cursor);
    }
    circ->read_cursor = 0;
    circ->write_cursor = length;
    free(circ->data);
    circ->data = new_data;
    circ->buffer_length = new_size;
}

void circular_buffer_shrink(
        circular_buffer * circ,
        uint32_t delta
        ) {
    //Will only work if we've got enough space..
    assert(!"Not yet implemented");
}

//TODO: What if the buffer is not empty?
void circular_buffer_free(
        circular_buffer * circ
        ) {
    assert((circular_buffer_size(circ)==0) && !"circular buffer not empty");
    free(circ->data);
    free(circ);
}
/* vim: set ts=2 sw=2 expandtab: */
