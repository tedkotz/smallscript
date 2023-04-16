#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "smallc.h"

#define SESC_TYPE_NONE          0
#define reference_ptr  intptr_t* restrict
#define reference_len       2
#define REFERENCE_INIT { SESC_TYPE_NONE, 0 }
/* intptr_t ref[] = REFERENCE_INIT;
 */

void reference_init( reference_ptr ref );
void reference_clear( reference_ptr ref );
void reference_copy( reference_ptr dst_ref, const reference_ptr src_ref );
void reference_move( reference_ptr dst_ref, reference_ptr src_ref);

void reference_create_bytes( reference_ptr ref, const char * bytes, intptr_t len );
void reference_create_str( reference_ptr ref, const char * str );
void reference_create_int( reference_ptr ref, intptr_t val );
void reference_create_bool( reference_ptr ref, intptr_t val );
void reference_create_obj( reference_ptr ref );
void reference_create_list( reference_ptr ref );

void reference_set_item( reference_ptr object_ref, const reference_ptr key_ref, const reference_ptr src_ref, char seed);
const reference_ptr reference_get_item( const reference_ptr object_ref, const reference_ptr key_ref, char seed);

intptr_t reference_extract_int(const reference_ptr ref );
intptr_t reference_extract_str( char* restrict buffer, intptr_t bufsz, const reference_ptr ref );


#endif /* HASHTABLE_H */
