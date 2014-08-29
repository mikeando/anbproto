//
//  tests/core/source.c
//
//  Created by Michael Anderson on 30/05/2014.
//  Copyright (c) 2014 Michael Anderson. All rights reserved.
//

#include <stdio.h>
#include <assert.h>

#include "clar/clar.h"

#include "anbutil/circular_buffer.h"


void test_core_dummy__capacity_is_correct(void) {

  circular_buffer * circ;

  circular_buffer_create(&circ, 10);

  int ok;
  void * dummy = &dummy;
  for(int i=0; i<10; ++i) {
    ok = circular_buffer_put(circ, dummy);
    cl_assert(ok==ECIRC_OK);
  }

  ok = circular_buffer_put(circ, dummy);
  cl_assert(ok==ECIRC_FULL);
}


void test_core_dummy__get_works(void) {

  circular_buffer * circ;

  circular_buffer_create(&circ, 10);

  void * dummy = &dummy;
  for(int i=0; i<5; ++i) {
    circular_buffer_put(circ, dummy);
  }

  int ok;
  void * result=NULL;
  for(int i=0; i<5; ++i) {
    result = NULL;
    ok = circular_buffer_get(circ, &result);
    cl_assert(ok==ECIRC_OK);
    cl_assert(result==dummy);
  }

  result = NULL;
  ok = circular_buffer_get(circ, &result);
  cl_assert(ok==ECIRC_EMPTY);
  cl_assert(result==NULL);
}

/**
 * Test that the size is correct in various situations
 */

void test_core_dummy__size_works(void) {

  circular_buffer * circ;
  circular_buffer_create(&circ, 10);

  /**
   * Empty at origin
   */
  circ->read_cursor = 0;
  circ->write_cursor = 0;
  cl_assert(circular_buffer_size(circ)==0);


  /**
   * Empty in middle 
   */
  circ->read_cursor = 5;
  circ->write_cursor = 5;
  cl_assert(circular_buffer_size(circ)==0);
  
  /**
   * Empty at end 
   */
  circ->read_cursor = 10;
  circ->write_cursor = 10;
  cl_assert(circular_buffer_size(circ)==0);

  /**
   * Full at origin
   */
  circ->read_cursor = 0;
  circ->write_cursor = 10;
  cl_assert(circular_buffer_size(circ)==10);

  /**
   * Full at +1
   */
  circ->read_cursor = 1;
  circ->write_cursor = 0;
  cl_assert(circular_buffer_size(circ)==10);

  /**
   * Full at middle
   */
  circ->read_cursor = 5;
  circ->write_cursor = 4;
  cl_assert(circular_buffer_size(circ)==10);

  /**
   * Full at end
   */
  circ->read_cursor = 10;
  circ->write_cursor = 9;
  cl_assert(circular_buffer_size(circ)==10);

  /**
   * half at origin
   */
  circ->read_cursor = 0;
  circ->write_cursor = 5;
  cl_assert(circular_buffer_size(circ)==5);

  /**
   * half at +1
   */
  circ->read_cursor = 1;
  circ->write_cursor = 6;
  cl_assert(circular_buffer_size(circ)==5);

  /**
   * half at middle
   */
  circ->read_cursor = 5;
  circ->write_cursor = 10;
  cl_assert(circular_buffer_size(circ)==5);

  circ->read_cursor = 5;
  circ->write_cursor = 0;
  cl_assert(circular_buffer_size(circ)==6);

  /**
   * half at end
   */
  circ->read_cursor = 10;
  circ->write_cursor = 4;
  cl_assert(circular_buffer_size(circ)==5);

}



/* vim: set ts=2 sw=2 expandtab: */
