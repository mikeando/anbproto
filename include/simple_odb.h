#ifndef SIMPLE_ODB_H
#define SIMPLE_ODB_H

#include "odb.h"
#include "anbproto/logger.h"

odb * simple_odb_init( logger * root_logger );
void simple_odb_destroy(odb* db);

#endif
