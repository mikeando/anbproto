//
//  tests/core/source.c
//
//  Created by Michael Anderson on 30/05/2014.
//  Copyright (c) 2014 Michael Anderson. All rights reserved.
//

#include <stdio.h>
#include <assert.h>

#include "clar/clar.h"


void test_core_dummy__should_fail(void) {
  cl_fail("Oh noes its an expected fail");
}
