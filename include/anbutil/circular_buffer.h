#ifndef ANBPROTO_CIRCULAR_BUFFER_H
#define ANBPROTO_CIRCULAR_BUFFER_H

#include <stdint.h>

struct circular_buffer {
    uint32_t read_cursor;
    uint32_t write_cursor;
    uint32_t buffer_length;
    void ** data;
};

typedef struct circular_buffer circular_buffer;

void circular_buffer_create(
        circular_buffer ** circ,
        uint32_t initial_capacity );

uint32_t circular_buffer_size(
        circular_buffer * circ
        );

void* circular_buffer_get(
        circular_buffer * circ
        );

void circular_buffer_put(
        circular_buffer * circ,
        void * p
        );

void * circular_buffer_peek(
        circular_buffer * circ 
        );

void circular_buffer_expand(
        circular_buffer * circ,
        uint32_t delta 
        );

void circular_buffer_shrink(
        circular_buffer * circ,
        uint32_t delta
        );

//TODO: What if the buffer is not empty?
void circular_buffer_free(
        circular_buffer * circ
        );

#endif

