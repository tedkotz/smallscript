#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "smallc.h"

#define reference_ptr  intptr_t*
#define REFERENCE_INIT { SESC_TYPE_NONE, 0 }
// intptr_t ref[] = REFERENCE_INIT;

void reference_clear( reference_ptr ref );
void reference_copy( reference_ptr dst_ref, const reference_ptr src_ref );
void reference_move( reference_ptr dst_ref, reference_ptr src_ref);

void reference_create_bytes( reference_ptr ref, const char * bytes, intptr_t len );
void reference_create_str( reference_ptr ref, const char * str );
void reference_create_int( reference_ptr ref, intptr_t val );
void reference_create_bool( reference_ptr ref, intptr_t val );
void reference_create_obj( reference_ptr ref );
void reference_create_list( reference_ptr ref );



#endif // HASHTABLE_H
