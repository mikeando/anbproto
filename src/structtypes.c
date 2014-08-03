#include "anbproto/structtypes.h"

#include <stdio.h>
#include <assert.h>

static const char hexchar[16] = "0123456789abcdef";

static char * hexify_uint64(uint64_t value, char* buffer) {

	int i;
	for(i=7;i>=0;--i) {
		unsigned char ac = value & 0xFF;
		value = value/256;
		buffer[2*i] = hexchar[ac/16];
		buffer[2*i+1] = hexchar[ac%16];
	}
	buffer[16] = 0;
	return buffer;
}

void check_struct_type_f( const char * type, const char* expr, uint64_t expected_magic, uint64_t magic, void * v, int line, const char * file) {
	if(v==NULL) {
		fprintf(stderr,"%s:%d %s isn't of type %s as it is NULL\n", file, line, expr, type);
		assert(0);
		return;
	}
	if(magic!=expected_magic) {
		char actual_str[17];
		char expected_str[17];
		fprintf(
				stderr, 
				"%s:%d: %s isn't of type %s : magic doesn't match - expected %s got %s\n", 
				file, 
				line,
				expr,
				type,
				hexify_uint64(expected_magic, expected_str), 
				hexify_uint64(magic, actual_str)
		       );
		assert(expected_magic==magic);
	}
}
