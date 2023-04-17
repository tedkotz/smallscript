

/* Includes ******************************************************************/
#include "hashtable.h"


/* Defines *******************************************************************/
#define NONE_HASH 0

/*
 * enum SESC_TYPE
 *#define SESC_TYPE_NONE          0
 */
#define SESC_TYPE_BOOL          1
#define SESC_TYPE_INT           2
#define SESC_TYPE_STR           3
#define SESC_TYPE_BYTES         4
#define SESC_TYPE_OBJ           5
#define SESC_TYPE_LIST          6
#define SESC_TYPE_FUNC          7
#define SESC_TYPE_CFUNC         8

/*
 *        case SESC_TYPE_NONE  :
 *        case SESC_TYPE_BOOL  :
 *        case SESC_TYPE_INT   :
 *        case SESC_TYPE_STR   :
 *        case SESC_TYPE_BYTES :
 *        case SESC_TYPE_OBJ   :
 *        case SESC_TYPE_LIST  :
 *        case SESC_TYPE_FUNC  :
 *        case SESC_TYPE_CFUNC :
 */

#define SESC_TYPE_STATIC_STR    0x12
#define SESC_TYPE_SUBTABLE      0x11
#define SESC_TYPE intptr_t

#define NOT_A_VLA_SIZE 64

/* Types *********************************************************************/
/* struct refable
 */
#define refable_ptr           intptr_t*
#define refable_refcnt_idx    0
#define refable_data_idx      1
#define refable_header_size   (refable_data_idx*SIZEOFINT)

/*
 * struct refable_str or refable_bytes {
 *     intptr_t refcnt
 *     intptr_t len
 *     char     data[]
 * }
 */
#define refable_bytes_len_idx       refable_data_idx
#define refable_bytes_data_idx      (refable_data_idx+1)
#define refable_bytes_header_size   (refable_bytes_data_idx*SIZEOFINT)

/*
 * struct sesc_reference {
 *     intptr_t header
 *     intptr_t dataptr
 * }
 */
#define ref_val_head        0
#define ref_val_data        1
#define reference_size     (reference_len*SIZEOFINT)

/*
 * struct ht_item {
 *     sesc_reference key
 *     sesc_reference value
 * }
 */
#define ht_item_ptr intptr_t*
#define ht_item_len 4
#define ht_item_size (ht_item_len*SIZEOFINT)
#define ht_item_key_head  0
#define ht_item_key_data  1
#define ht_item_val_head  2
#define ht_item_val_data  3

/*
 * struct refable_list or refable_object {
 *     intptr_t refcnt
 *     ht_item  table[htable_len]
 * }
 * CONFIG hash_bits should work from 2-8
 * 2 showed the minimum allocation for my large data sets
 * 8 was the fastest about 3x on my system
 * 3 was only slightly larger with a good bump in speed for "normal" data sets
 * Try profiling on your system with your data sets.
 */
#define hash_bits 3
#define htable_len  (1 << hash_bits)
#define refable_htable_data_idx  refable_data_idx
#define refable_htable_idx_cnt   (refable_data_idx+(htable_len*ht_item_len))
#define refable_htable_size      (refable_htable_idx_cnt*SIZEOFINT)

/*
 * struct sesc_context {
 *     sesc_reference current_scope
 *     ??sesc_reference stack_list
 *     ??intptr_t       stack_ptr
 *  };
 * Calling conventions
 * The calling object is passed as the entries "0"
 * args are pushed as numbered entries left to right starting at 1
 * obj.method(a, b, c) -> [0] = obj, [1]=a, [2]=b, [3]=c
 * return values are stored as negative numbered entries left to right starting at -1
 * key "@parent" is next scope up.
 * key "@root" is top scope or is None if this is the global scope
 */
#define sesc_context_ptr  reference_ptr
#define sesc_context_len  reference_len
#define sesc_context_size reference_size

#define sesc_attr_ptr intptr_t*

/* Interfaces ****************************************************************/
typedef void (*SESC_CFUNC_TYPE)(reference_ptr ctx);


/* Data **********************************************************************/
const intptr_t REFERENCE_NONE[] = REFERENCE_INIT;
const intptr_t REFERENCE_ZERO[] = { SESC_TYPE_INT, 0 };
const intptr_t REFERENCE_1[] = { SESC_TYPE_INT, 1 };
const intptr_t REFERENCE_2[] = { SESC_TYPE_INT, 2 };
const intptr_t REFERENCE_3[] = { SESC_TYPE_INT, 3 };
const intptr_t REFERENCE_n1[] = { SESC_TYPE_INT, -1 };
const intptr_t REFERENCE_n2[] = { SESC_TYPE_INT, -2 };
const intptr_t REFERENCE_n3[] = { SESC_TYPE_INT, -3 };


intptr_t max_seed = 0;


/* Functions *****************************************************************/

/** String manipulation ******************************************************/
bool str_containschar(const char * restrict str, char c )
{
    while( *str != '\0' )
    {
        if (*str++ == c)
        {
            return true;
        }
    }
    return false;
}

char * str_ltrim( char * restrict str, const char * restrict delim )
{
    while( str_containschar( delim, *str) )
    {
        ++str;
    }
    if (*str == '\0')
    {
        return NULL;
    }
    return str;
}

char * str_tail( char * restrict head, const char * restrict delim )
{
    while( !str_containschar(delim, *head) )
    {
        if (*head++ == '\0')
        {
            return NULL;
        }
    }
    *head++ = '\0';
    return head;
}

/**
 * str_sncpy is like strncpy, with snprintf semantics.
 * Thus it is very similar to snprintf( dst, dst_max_sz, "%"src_max_sz"s", src )
 *
 * @param dst pointer to the character array to copy to
 * @param dst_max_sz the size of the destination buffer
 * @param src pointer to the character array to copy from
 * @param src_max_sz maximum number of characters in src, ignored if negative
 *
 * @return number of characters (not including the terminating null character)
 * which would have been written to buffer if dst_max_sz was ignored.
 */
intptr_t str_sncpy ( char* restrict dst, intptr_t dst_max_sz, const char* restrict src, intptr_t src_max_sz )
{
    intptr_t count = 0;
    const intptr_t max_copies = (dst == NULL) ? 0 :
        (((src_max_sz >= 0) && (src_max_sz < dst_max_sz)) ? src_max_sz : (dst_max_sz-1));

    while( (count<max_copies)  &&
           (*src != '\0') )
    {
        *dst++ = *src++;
        ++count;
    }
    if( max_copies > 0 )
    {
        *dst='\0';
    }
    if(src_max_sz >= 0)
    {
        while( (count<src_max_sz) &&
               (*src++ != '\0') )
        {
            ++count;
        }
    }
    else
    {
        while(*src++ != '\0')
        {
            ++count;
        }
    }
    return count;
}


/** Reference Creation Functions *********************************************/
void refable_header_init( refable_ptr data )
{
   data[refable_refcnt_idx]=0;
}

void reference_init( reference_ptr ref )
{
    ref[ref_val_head]=SESC_TYPE_NONE;
    ref[ref_val_data]=0;
}

/** Reference Destroy Functions **********************************************/
void reference_clear( reference_ptr ref )
{
    switch (ref[ref_val_head])
    {
        case SESC_TYPE_OBJ   :
        case SESC_TYPE_LIST  :
        case SESC_TYPE_SUBTABLE:
        {
            refable_ptr tmp = (refable_ptr)ref[ref_val_data];
            --(tmp[refable_refcnt_idx]);
            if(tmp[refable_refcnt_idx] < 1)
            {
                ht_item_ptr item=&tmp[refable_htable_data_idx];
                intptr_t i=htable_len;
                while( i-- > 0 )
                {
                    reference_clear(&item[ht_item_key_head]);
                    reference_clear(&item[ht_item_val_head]);
                    item += ht_item_len;
                }
                free(tmp);
            }
            break;
        }
        /*
         * case SESC_TYPE_FUNC  :
         * {
         *     refable_ptr tmp = (refable_ptr)ref[ref_val_data];
         *     --(tmp[refable_refcnt_idx]);
         *     if(tmp[refable_refcnt_idx] < 1)
         *     {
         *         func_destroy(tmp);
         *     }
         *     break;
         * }
         */

        case SESC_TYPE_STR   :
        case SESC_TYPE_BYTES :
        {
            /* Decrement ref count then free */
            refable_ptr tmp = (refable_ptr)ref[ref_val_data];
            --(tmp[refable_refcnt_idx]);
            printf("ref count = %d\n", (int)tmp[refable_refcnt_idx]);
            if(tmp[refable_refcnt_idx] < 1)
            {
                free(tmp);
            }
            break;
        }
        /*
         * case SESC_TYPE_NONE  :
         * case SESC_TYPE_BOOL  :
         * case SESC_TYPE_INT   :
         * case SESC_TYPE_CFUNC :
         */
    }
    ref[ref_val_head]=SESC_TYPE_NONE;
    ref[ref_val_data]=0;
}

void reference_set_refable( reference_ptr dst_ref, SESC_TYPE val_head, refable_ptr val_data  )
{
    if( dst_ref[ref_val_head]!=val_head ||
        dst_ref[ref_val_data]!=(intptr_t)val_data )
    {
        switch (val_head)
        {
            case SESC_TYPE_STR   :
            case SESC_TYPE_BYTES :
            case SESC_TYPE_OBJ   :
            case SESC_TYPE_LIST  :
            case SESC_TYPE_SUBTABLE:
            /* case SESC_TYPE_FUNC  :
             */
                ++(val_data[refable_refcnt_idx]);
                break;
            /*
             * case SESC_TYPE_NONE  :
             * case SESC_TYPE_BOOL  :
             * case SESC_TYPE_INT   :
             * case SESC_TYPE_CFUNC :
             */
        }

        reference_clear(dst_ref);
        dst_ref[ref_val_head]=val_head;
        dst_ref[ref_val_data]=(intptr_t)val_data;
    }
}

void reference_copy( reference_ptr dst_ref, const reference_ptr src_ref )
{
    if( dst_ref!=src_ref )
    {
        reference_set_refable( dst_ref, src_ref[ref_val_head], (refable_ptr)src_ref[ref_val_data] );
    }
}

void reference_move( reference_ptr dst_ref, reference_ptr src_ref)
{
    if( dst_ref!=src_ref )
    {
        reference_clear(dst_ref);
        dst_ref[ref_val_head]=src_ref[ref_val_head];
        dst_ref[ref_val_data]=src_ref[ref_val_data];
        src_ref[ref_val_head]=SESC_TYPE_NONE;
        src_ref[ref_val_data]=0;
    }
}


/** Reference Create Functions ***********************************************/
void reference_create_byteslike( reference_ptr ref, const char * bytes, intptr_t len, SESC_TYPE val_head)
{
    refable_ptr data = malloc(refable_bytes_header_size+len);
    if( data != NULL )
    {
        refable_header_init( data );
        data[refable_bytes_len_idx] = len;
        memcpy ( (char*)&data[refable_bytes_data_idx], bytes, len);
        reference_set_refable(  ref, val_head, data );
    }
    else
    {
        reference_clear(ref);
    }
}

void reference_create_bytes( reference_ptr ref, const char * bytes, intptr_t len )
{
    return reference_create_byteslike( ref, bytes, len, SESC_TYPE_BYTES);
}

void reference_create_str( reference_ptr ref, const char * str )
{
    return reference_create_byteslike( ref, str, strlen(str)+1, SESC_TYPE_STR);
}

void reference_create_int( reference_ptr ref, intptr_t val )
{
    reference_clear(ref);
    ref[ref_val_head]=SESC_TYPE_INT;
    ref[ref_val_data]=val;
}

void reference_create_bool( reference_ptr ref, intptr_t val )
{
    reference_clear(ref);
    ref[ref_val_head]=SESC_TYPE_BOOL;
    ref[ref_val_data]=(0!=val);
}

void reference_create_cfunc( reference_ptr ref, SESC_CFUNC_TYPE func )
{
    reference_clear(ref);
    ref[ref_val_head]=SESC_TYPE_CFUNC;
    ref[ref_val_data]=(intptr_t)func;
}

void reference_create_obj( reference_ptr ref )
{
    reference_set_refable(ref, SESC_TYPE_OBJ, calloc( refable_htable_size, 1) );
}

void reference_create_list( reference_ptr ref )
{
    reference_set_refable(ref, SESC_TYPE_LIST, calloc( refable_htable_size, 1) );
}

void reference_create_subtable( reference_ptr ref )
{
    reference_set_refable(ref, SESC_TYPE_SUBTABLE, calloc( refable_htable_size, 1) );
}

/** Reference Extract Functions **********************************************/
const char* reference_extract_bytes( const reference_ptr ref, intptr_t* return_len)
{
    switch (ref[ref_val_head])
    {
        case SESC_TYPE_NONE  :
            *return_len = 0;
            return NULL;

        case SESC_TYPE_BOOL  :
        case SESC_TYPE_INT   :
        case SESC_TYPE_OBJ   :
        case SESC_TYPE_LIST  :
        case SESC_TYPE_FUNC  :
        case SESC_TYPE_CFUNC  :
        {
            *return_len = SIZEOFINT;
            return (char*)&ref[ref_val_data];
        }

        case SESC_TYPE_STR   :
        case SESC_TYPE_BYTES :
        {
            refable_ptr data=(refable_ptr)ref[ref_val_data];
            *return_len = data[refable_bytes_len_idx];
            return (char*)&data[refable_bytes_data_idx];
        }

    }
    fputs("reference_extract_bytes():Unknown Type", stderr);
    *return_len = reference_size;
    return (char*)ref;
}

/**
 * Copies the string representation of reference to buffer with snprintf semantics.
 *
 * @param buffer pointer to the character array to copy to
 * @param bufsz the size of the destination buffer
 * @param ref pointer to the reference to build string for
 *
 * @return number of characters (not including the terminating null character)
 * which would have been written to buffer if bufsz was ignored, or a negative
 * value if an encoding error (for string and character conversion specifiers) occurred
 */
intptr_t reference_extract_str( char* restrict buffer, intptr_t bufsz, const reference_ptr ref )
{
    switch (ref[ref_val_head])
    {
        case SESC_TYPE_NONE:
        {
            return str_sncpy(buffer, bufsz, "None", -1);
        }

        case SESC_TYPE_BOOL  :
        {
            if(ref[ref_val_data])
            {
                return str_sncpy(buffer, bufsz, "True", -1);
            }
            else
            {
                return str_sncpy(buffer, bufsz, "False", -1);
            }
        }

        case SESC_TYPE_INT   :
        {
            return snprintf(buffer, bufsz, "%"PRIdPTR, ref[ref_val_data] );
        }
        case SESC_TYPE_STR   :
        {
            refable_ptr data=(refable_ptr)ref[ref_val_data];
            return str_sncpy(buffer, bufsz,
                           (char*)&data[refable_bytes_data_idx],
                           data[refable_bytes_len_idx]);
        }
        case SESC_TYPE_BYTES :
        {
            return str_sncpy(buffer, bufsz, "bytes", -1);
        }
        case SESC_TYPE_OBJ   :
        {
            return str_sncpy(buffer, bufsz, "object", -1);
        }
        case SESC_TYPE_LIST  :
        {
            return str_sncpy(buffer, bufsz, "list", -1);
        }
        case SESC_TYPE_FUNC  :
        {
            return str_sncpy(buffer, bufsz, "func", -1);
        }
        case SESC_TYPE_CFUNC  :
        {
            return str_sncpy(buffer, bufsz, "extern", -1);
        }
    }
    fputs("reference_extract_str():Unknown Type", stderr);
    return -1;
}

intptr_t reference_extract_int(const reference_ptr ref )
{
    return ref[ref_val_data];
}

intptr_t reference_extract_bool(const reference_ptr ref )
{
    return (0!=ref[ref_val_data]);
}

void reference_deep_copy( reference_ptr dst_ref, const reference_ptr src_ref )
{
    if( dst_ref!=src_ref )
    {
        switch (src_ref[ref_val_head])
        {
            case SESC_TYPE_NONE  :
                return reference_clear(dst_ref);

            case SESC_TYPE_BOOL  :
            case SESC_TYPE_INT   :
                /* shallow copy works fine */
                return reference_copy( dst_ref, src_ref );

            case SESC_TYPE_OBJ   :
            case SESC_TYPE_LIST  :
            case SESC_TYPE_FUNC  :
            {
                /*
                 * TKOTZ - copy the objects someday?
                 * shallow copy works fine?
                 */
                return reference_copy( dst_ref, src_ref );
            }

            case SESC_TYPE_STR   :
            case SESC_TYPE_BYTES :
            {
                /* copy the data */
                /* refable_ptr data=(refable_ptr)ref[ref_val_data] */
                intptr_t len;
                const char * bytes = reference_extract_bytes(src_ref, &len);
                return reference_create_byteslike(dst_ref, bytes, len, src_ref[ref_val_head]);
            }
        }
    }
}

intptr_t reference_cmp( const reference_ptr l_ref, const reference_ptr r_ref )
{
    if(l_ref[ref_val_head] != r_ref[ref_val_head])
    {
        return -100;
    }
    switch (l_ref[ref_val_head])
    {
        case SESC_TYPE_NONE  :
            return 0;

        case SESC_TYPE_BOOL  :
        case SESC_TYPE_INT   :
        case SESC_TYPE_OBJ   :
        case SESC_TYPE_LIST  :
        case SESC_TYPE_FUNC  :
        case SESC_TYPE_CFUNC  :
            return reference_extract_int(r_ref) - reference_extract_int(l_ref);

        case SESC_TYPE_STR   :
        case SESC_TYPE_BYTES :
        {
           refable_ptr r_data=(refable_ptr)r_ref[ref_val_data];
           refable_ptr l_data=(refable_ptr)l_ref[ref_val_data];
           intptr_t r_len = r_data[refable_bytes_len_idx];
           if( r_len != l_data[refable_bytes_len_idx] )
           {
               return -100;
           }
           else
           {
               return memcmp((char*)&r_data[refable_bytes_data_idx], (char*)&l_data[refable_bytes_data_idx], r_len);
           }
        }
    }
    fputs("reference_cmp():Unknown Type", stderr);
    return -100;
}

void reference_call( reference_ptr parent_ctx, const reference_ptr callable/*, reference_ptr arg_list */)
{
    switch (callable[ref_val_head])
    {
        case SESC_TYPE_CFUNC  :
        {
            SESC_CFUNC_TYPE func = (SESC_CFUNC_TYPE)callable[ref_val_data];
            func( parent_ctx );
        }
        /*
         * case SESC_TYPE_NONE  :
         * case SESC_TYPE_BOOL  :
         * case SESC_TYPE_INT   :
         * case SESC_TYPE_STR   :
         * case SESC_TYPE_BYTES :
         * case SESC_TYPE_OBJ   :
         * case SESC_TYPE_LIST  :
         * case SESC_TYPE_FUNC  :
         *    intptr_t ctx[] = REFERENCE_INIT;
         *    reference_create_object(ctx);
         */
    }
}


/** Hash Table Functions *****************************************************/

static unsigned char hash_bytes( const char* data, intptr_t len, char seed )
{
    /*
     * Need a good 8-bit hash value. Let's try the family of CRCs.
     * So let's use a 10-bit polynomial with the 8 seed bits being the middle terms
     * Note the terms here are bit 6 represents x^0 and
     * x^9 is assumed to be one by checking it before the shift
     * poly    15  14  13  12  11  10  09  08  07  06  05  04  03  02  01  00
     * seed        b7  b6  b5  b4  b3  b2  b1  b0
     * 513<<6  1                                   1
     * term    x^9 x^8 x^7 x^6 x^5 x^4 x^3 x^2 x^1 x^0
     */
    intptr_t poly = (513 << 6) + (seed << 7) ;

    intptr_t accum = 0xFF00;
    char i;
    while( len-- > 0)
    {
        accum ^= (*data++ & 0x0FF);
        i=8;
        while(i-- > 0)
        {
            /* Might be faster on sys without native multiply or branch prediction
             * if( accum & 0x8000 ) accum ^= poly;
             */
            accum ^= (poly * ((accum >> 15) & 1));
            accum <<= 1;
        }
    }
    i=8;
    while(i-- > 0)
    {
        /* Might be faster on sys without native multiply or branch prediction
         * if( accum & 0x8000 ) accum ^= poly;
         */
        accum ^= (poly * ((accum >> 15) & 1));
        accum <<= 1;
    }
    return (accum >> 7) & (htable_len-1);
}

unsigned char reference_hash( const reference_ptr ref, char seed )
{
    char type = ref[ref_val_head];
    seed += type;
    switch (type)
    {
        case SESC_TYPE_NONE  :
            return NONE_HASH;

        case SESC_TYPE_BOOL  :
        case SESC_TYPE_INT   :
        case SESC_TYPE_OBJ   :
        case SESC_TYPE_LIST  :
        case SESC_TYPE_FUNC  :
        case SESC_TYPE_CFUNC :
        {
            /* hash the reference */
            return hash_bytes( (const char*)ref, reference_size, seed);
        }

        case SESC_TYPE_STR   :
        case SESC_TYPE_BYTES :
        {
            /* hash the data  */
            refable_ptr data=(refable_ptr)ref[ref_val_data];
            return hash_bytes( (char*)&data[refable_bytes_data_idx], data[refable_bytes_len_idx], seed);
        }
    }
    fputs("reference_hash():Unknown Type", stderr);
    /* hash the reference */
    return hash_bytes( (const char*)ref, reference_size, seed );
}

void reference_set_item( reference_ptr object_ref, const reference_ptr key_ref, const reference_ptr src_ref, char seed)
{
    refable_ptr data = NULL;
    switch (object_ref[ref_val_head])
    {
        case SESC_TYPE_OBJ   :
        case SESC_TYPE_LIST  :
            seed = 0;
            /* intentional fall thru */
        case SESC_TYPE_SUBTABLE:
            data=(refable_ptr)object_ref[ref_val_data];
            break;
        /*
         * case SESC_TYPE_NONE  :
         * case SESC_TYPE_BOOL  :
         * case SESC_TYPE_INT   :
         * case SESC_TYPE_STR   :
         * case SESC_TYPE_BYTES :
         * case SESC_TYPE_FUNC  :
         * case SESC_TYPE_CFUNC :
         */
        default:
        return;
    }
    unsigned hash = reference_hash(key_ref, seed);
    ht_item_ptr entry = data + (refable_htable_data_idx + ht_item_len * hash);

    if (entry[ht_item_val_head] == SESC_TYPE_NONE)
    {
        /* Empty */
        if( src_ref[ref_val_head] != SESC_TYPE_NONE )
        {
            reference_copy( &entry[ht_item_val_head], src_ref);
            reference_deep_copy( &entry[ht_item_key_head], key_ref);
        }
    }
    else if (entry[ht_item_val_head] == SESC_TYPE_SUBTABLE)
    {
        /* check subtable */
        return reference_set_item( &entry[ht_item_val_head], key_ref, src_ref, seed+1);
    }
    else if (0 == reference_cmp(key_ref, &entry[ht_item_key_head]))
    {
        /* Replace */
        reference_copy( &entry[ht_item_val_head], src_ref);
    }
    else if (seed+1 != 0)
    {
        /* Collision */
        if (seed > max_seed)
        {
            max_seed = seed;
            printf("Max Seed %d\n", (int)max_seed);
        }
        intptr_t new_table[] = REFERENCE_INIT;

        reference_create_subtable( new_table);

        reference_set_item( new_table, &entry[ht_item_key_head], &entry[ht_item_val_head], seed+1);

        reference_move(&entry[ht_item_val_head], new_table);

        reference_clear( &entry[ht_item_key_head] );

        return reference_set_item( &entry[ht_item_val_head], key_ref, src_ref, seed+1);
    }
    else
    {
        fputs("htable_set_item():Failed to store", stderr);
    }
}


const reference_ptr reference_get_item( const reference_ptr object_ref, const reference_ptr key_ref, char seed)
{
    refable_ptr data = NULL;
    switch (object_ref[ref_val_head])
    {
        case SESC_TYPE_OBJ   :
        case SESC_TYPE_LIST  :
            seed = 0;
            /* intentional fall thru */
        case SESC_TYPE_SUBTABLE:
            data=(refable_ptr)object_ref[ref_val_data];
            break;
        /*
         * case SESC_TYPE_NONE  :
         * case SESC_TYPE_BOOL  :
         * case SESC_TYPE_INT   :
         * case SESC_TYPE_STR   :
         * case SESC_TYPE_BYTES :
         * case SESC_TYPE_FUNC  :
         * case SESC_TYPE_CFUNC :
         */
        default:
        return REFERENCE_NONE;
    }
    unsigned hash = reference_hash(key_ref, seed);
    ht_item_ptr entry = data + (refable_htable_data_idx + ht_item_len * hash);

    if (entry[ht_item_val_head] == SESC_TYPE_SUBTABLE)
    {
        /* Check subtable */
        return reference_get_item( &entry[ht_item_val_head], key_ref, seed+1);
    }
    else if (0 == reference_cmp(key_ref, &entry[ht_item_key_head]))
    {
        /* Found */
        return &entry[ht_item_val_head];
    }
    else
    {
        return REFERENCE_NONE;
    }
}

/*****************************************************************************/
/** Context and Calling ******************************************************/
void cfunc_add(reference_ptr ctx)
{
    intptr_t l = reference_extract_int(reference_get_item(ctx, REFERENCE_1, 0));
    intptr_t r = reference_extract_int(reference_get_item(ctx, REFERENCE_2, 0));
    intptr_t ref[] = REFERENCE_INIT;
    reference_create_int( ref, l+r );
    reference_set_item( ctx, REFERENCE_n1, ref, 0 );
}

void cfunc_sub(reference_ptr ctx)
{
    intptr_t l = reference_extract_int(reference_get_item(ctx, REFERENCE_1, 0));
    intptr_t r = reference_extract_int(reference_get_item(ctx, REFERENCE_2, 0));
    intptr_t ref[] = REFERENCE_INIT;
    reference_create_int( ref, l-r );
    reference_set_item( ctx, REFERENCE_n1, ref, 0 );
}

void cfunc_print(reference_ptr ctx)
{
    char buff[NOT_A_VLA_SIZE];
    char * str=buff;
    const reference_ptr str_ref = reference_get_item(ctx, REFERENCE_1, 0 );
    intptr_t concat = reference_extract_int(reference_get_item(ctx, REFERENCE_2, 0));
    intptr_t size = reference_extract_str(str, NOT_A_VLA_SIZE, str_ref );
    /* plus 2 : 1 for the null and one optionally for the newline */
    intptr_t str_size = size + 2 ;
    if( str_size > NOT_A_VLA_SIZE )
    {
        str = malloc(str_size);
        size = reference_extract_str(str, str_size, str_ref );
    }
    if( !concat )
    {
        str[size++]='\n';
        str[size++]='\0';
    }
    fputs( str, stdout );
    if( str != buff )
    {
        free(str);
    }
}

void cfunc_input(reference_ptr ctx)
{
    intptr_t ref[] = REFERENCE_INIT;
    char buff[256];
    fgets( buff, 256, stdin );
    str_tail( buff, "\n\r");
    reference_create_str( ref, buff);
    reference_set_item( ctx, REFERENCE_n1, ref, 0 );
}

void cfunc_inputint(reference_ptr ctx)
{
    intptr_t ref[] = REFERENCE_INIT;
    char buff[256];
    fgets( buff, 256, stdin );
    reference_create_int( ref, atoi(buff));
    reference_set_item( ctx, REFERENCE_n1, ref, 0 );
}

sesc_attr_ptr sesc_attr_create(void)
{
    return NULL;
}

void sesc_attr_destroy(unused sesc_attr_ptr ctx)
{
}

sesc_context_ptr sesc_context_create(unused sesc_attr_ptr attr)
{
    intptr_t ref[] = REFERENCE_INIT;
    intptr_t key_ref[] = REFERENCE_INIT;
    sesc_context_ptr ctx = calloc( sesc_context_size, 1);
    reference_create_obj(ctx);

    reference_create_str( key_ref, "add" );
    reference_create_cfunc( ref, cfunc_add );
    reference_set_item( ctx, key_ref, ref, 0 );

    reference_create_str( key_ref, "sub" );
    reference_create_cfunc( ref, cfunc_sub );
    reference_set_item( ctx, key_ref, ref, 0 );

    reference_create_str( key_ref, "print" );
    reference_create_cfunc( ref, cfunc_print );
    reference_set_item( ctx, key_ref, ref, 0 );

    reference_create_str( key_ref, "input" );
    reference_create_cfunc( ref, cfunc_input );
    reference_set_item( ctx, key_ref, ref, 0 );

    reference_create_str( key_ref, "inputint" );
    reference_create_cfunc( ref, cfunc_inputint );
    reference_set_item( ctx, key_ref, ref, 0 );

    reference_clear( ref );
    reference_clear( key_ref );
    return ctx;
}

const reference_ptr walk_scope_get_item( const reference_ptr ctx, const reference_ptr key_ref)
{
    const reference_ptr returnVal = reference_get_item(ctx, key_ref, 0);
    if( 0 == reference_cmp(returnVal, REFERENCE_NONE) )
    {
        const reference_ptr parent_ref = reference_get_item(ctx, REFERENCE_ZERO, 0);
        if(parent_ref[ref_val_head] == SESC_TYPE_OBJ)
        {
            return walk_scope_get_item( parent_ref, key_ref);
        }
    }
    return returnVal;
}

void sesc_eval_argument( reference_ptr ref, sesc_context_ptr ctx, char * str)
{
    intptr_t key_ref[] = REFERENCE_INIT;
    /* Supports string["'], int[-+0-9], variable[_a-zA-Z] */
    switch ( str[0] )
    {
        case '+':
        case '-':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return reference_create_int( ref, atoi(str) );


        case '\"':
            ++str;
            str_tail( str, "\"" );
            return reference_create_str( ref, str );

        case '\'':
            ++str;
            str_tail( str, "\'" );
            return reference_create_str( ref, str );

        default:
            if( (str[0] == '_') ||
                (str[0] >= 'a' && str[0] <= 'z') ||
                (str[0] >= 'A' && str[0] <= 'Z')
            )
            {
                reference_create_str( key_ref, str );
                return reference_copy( ref, walk_scope_get_item(ctx, key_ref));
            }
    }
    return reference_clear(ref);
}

intptr_t sesc_eval_string(sesc_context_ptr ctx, const char * str)
{
    /* TODO
     * simple expression
     * ([_a-zA-z][_a-zA-z0-9]*)=([_a-zA-z][_a-zA-z0-9]*)\[[0-9]+(,[0-9]+)*\];
     */
    char buff[NOT_A_VLA_SIZE];
    char * tokenspace=buff;
    intptr_t idx = 1;
    intptr_t key_ref[] = REFERENCE_INIT;
    intptr_t ref[] = REFERENCE_INIT;
    const reference_ptr return_ref = REFERENCE_ZERO;
    intptr_t len = str_sncpy(tokenspace, NOT_A_VLA_SIZE, str, -1)+1;
    if( len > NOT_A_VLA_SIZE )
    {
        tokenspace = malloc(len);
        len = str_sncpy(tokenspace, len, str, -1)+1;
    }
    char* varname = str_ltrim(tokenspace, " ");
    char* funcname = str_tail( varname, "=" );
    if( NULL == funcname )
    {
        funcname =  varname;
        varname = NULL;
    }
    char* argument = str_tail( funcname, "(" );
    str_tail( argument, ")" );
    char* arguments = str_tail( argument, "," );
    while( NULL != arguments )
    {
        sesc_eval_argument( ref, ctx, argument);
        reference_create_int( key_ref, idx++ );
        reference_set_item( ctx, key_ref, ref, 0);
        argument = arguments;
        arguments = str_tail( argument, "," );
    }
    reference_create_int( ref, atoi(argument) );
    reference_create_int( key_ref, idx++ );
    reference_set_item( ctx, key_ref, ref, 0);

    reference_create_str( key_ref, funcname );
    reference_call( ctx, walk_scope_get_item(ctx, key_ref));
    if( varname != NULL )
    {
        reference_create_str( key_ref, varname );
        return_ref = reference_get_item(ctx, REFERENCE_n1, 0 );
        reference_set_item( ctx, key_ref, return_ref , 0);
    }
    reference_clear(key_ref);
    if( tokenspace!=buff )
    {
        free(tokenspace);
    }
    return reference_extract_int(return_ref);
}

intptr_t sesc_get_int_by_idx(sesc_context_ptr ctx, intptr_t idx)
{
    intptr_t key_ref[] = REFERENCE_INIT;
    reference_create_int( key_ref, idx );
    return reference_extract_int(reference_get_item(ctx, key_ref, 0));
    /* int references don't have to be cleaned up. */
}

intptr_t sesc_get_int_by_name(sesc_context_ptr ctx, const char* name)
{
    intptr_t key_ref[] = REFERENCE_INIT;
    reference_create_str( key_ref, name );
    intptr_t returnVal = reference_extract_int(walk_scope_get_item(ctx, key_ref));
    reference_clear(key_ref);
    return returnVal;
}

intptr_t sesc_get_string_by_idx(char* restrict buffer, intptr_t bufsz, sesc_context_ptr ctx, intptr_t idx)
{
    intptr_t key_ref[] = REFERENCE_INIT;
    reference_create_int( key_ref, idx );
    return reference_extract_str(buffer, bufsz, reference_get_item(ctx, key_ref, 0));
    /* int references don't have to be cleaned up. */
}

intptr_t sesc_get_string_by_name(char* restrict buffer, intptr_t bufsz, sesc_context_ptr ctx, const char* name)
{
    intptr_t key_ref[] = REFERENCE_INIT;
    reference_create_str( key_ref, name );
    intptr_t returnVal = reference_extract_str(buffer, bufsz, walk_scope_get_item(ctx, key_ref));
    reference_clear(key_ref);
    return returnVal;
}

void sesc_context_destroy(sesc_context_ptr ctx)
{
    reference_clear(ctx);
    free(ctx);
}






/* TODO - TKOTZ
 */

/*****************************************************************************/
/** Main     *****************************************************************/
/*****************************************************************************/
#ifdef HASHTEST
#include <malloc.h>
void main()
{
    const char * const strs[] = {
    "a"            ,
    "any"          ,
    "argc"         ,
    "argcount"     ,
    "args"         ,
    "argv"         ,
    "b"            ,
    "c"            ,
    "call"         ,
    "check"        ,
    "current"      ,
    "d"            ,
    "data"         ,
    "e"            ,
    "error"        ,
    "f"            ,
    "failed"       ,
    "file"         ,
    "g"            ,
    "i"            ,
    "index"        ,
    "input"        ,
    "instr"        ,
    "item"         ,
    "j"            ,
    "k"            ,
    "l"            ,
    "len"          ,
    "length"       ,
    "list"         ,
    "n"            ,
    "name"         ,
    "no"           ,
    "none"         ,
    "ok"           ,
    "one"          ,
    "output"       ,
    "p"            ,
    "parameter"    ,
    "point"        ,
    "position"     ,
    "prob"         ,
    "ptr"          ,
    "ref"          ,
    "report"       ,
    "result"       ,
    "right"        ,
    "s"            ,
    "set"          ,
    "state"        ,
    "status"       ,
    "t"            ,
    "table"        ,
    "test"         ,
    "text"         ,
    "this"         ,
    "tmp"          ,
    "unit"         ,
    "used"         ,
    "val"          ,
    "value"        ,
    "values"       ,
    "x"            ,
    "xt"           ,
    "y"            ,
    "zero"         ,
    "ze"           ,
    ""
    };

    char st1[] = "Hello";
    char st2[] = "This";
    char st3[] = "That";
    char st4[] = "Fing";
    char st5[] = "Fang";
    char st6[] = "Foom";
    intptr_t i=0;
    struct mallinfo mi;
    const int num_strs = sizeof(strs)/sizeof(strs[0]);
    const reference_ptr refptr;
    intptr_t ref[] = REFERENCE_INIT;
    intptr_t key_ref[] = REFERENCE_INIT;
    intptr_t htable[]  = REFERENCE_INIT;
    intptr_t sizeofoutputbuffer = 2;
    char * outputbuffer = NULL;
    intptr_t neededspace;


    for( i=0; i < num_strs; ++i )
    {
        int len = strlen(strs[i]);
        printf( "%02X : %02X : %02X : %s\n", (int)hash_bytes( strs[i], len, 0), (int)hash_bytes( strs[i], len, 1), (int)hash_bytes( strs[i], len, 2), strs[i]);
    }


    printf( "%s - %02X\n", st1, (int)hash_bytes( st1, sizeof(st1)-1, 0) & 0x0FF);
    printf( "%s - %02X\n", st2, (int)hash_bytes( st2, sizeof(st2)-1, 0) & 0x0FF);
    printf( "%s - %02X\n", st3, (int)hash_bytes( st3, sizeof(st3)-1, 0) & 0x0FF);
    printf( "%s - %02X\n", st4, (int)hash_bytes( st4, sizeof(st4)-1, 0) & 0x0FF);
    printf( "%s - %02X\n", st5, (int)hash_bytes( st5, sizeof(st5)-1, 0) & 0x0FF);
    printf( "%s - %02X\n", st6, (int)hash_bytes( st6, sizeof(st6)-1, 0) & 0x0FF);

    mi = mallinfo();
    printf("inuse blocks: %d\n", mi.uordblks);

    outputbuffer = malloc( sizeofoutputbuffer );

    mi = mallinfo();
    printf("inuse blocks: %d\n", mi.uordblks);

    reference_create_obj( htable );

    mi = mallinfo();
    printf("Created htable.\ninuse blocks: %d\n", mi.uordblks);

    for( i=3; i < num_strs; ++i )
    {
        reference_create_str( key_ref, strs[i] );
        reference_create_str( ref, strs[i-3] );
        reference_set_item( htable, key_ref, ref, 0 );
    }

    for( i=0; i < num_strs; ++i )
    {
        reference_create_str( key_ref, strs[i] );
        refptr = reference_get_item(htable, key_ref, 0 );
        neededspace = reference_extract_str(outputbuffer, sizeofoutputbuffer, refptr );
        if ( (neededspace + 1) > sizeofoutputbuffer )
        {
            sizeofoutputbuffer = neededspace * 2;
            outputbuffer = realloc ( outputbuffer, sizeofoutputbuffer);
            neededspace = reference_extract_str(outputbuffer, sizeofoutputbuffer, refptr );
        }
        printf( "a.%s = \"%s\"\n", strs[i],  outputbuffer);
    }

    for( i=5; i < num_strs; ++i )
    {
        reference_create_str( key_ref, strs[i] );
        reference_create_str( ref, strs[i-5] );
        reference_set_item( htable, key_ref, ref, 0 );
    }

    for( i=0; i < num_strs; ++i )
    {
        reference_create_str( key_ref, strs[i] );
        refptr = reference_get_item(htable, key_ref, 0 );
        neededspace = reference_extract_str(outputbuffer, sizeofoutputbuffer, refptr );
        if ( (neededspace + 1) > sizeofoutputbuffer )
        {
            sizeofoutputbuffer = neededspace * 2;
            outputbuffer = realloc ( outputbuffer, sizeofoutputbuffer);
            neededspace = reference_extract_str(outputbuffer, sizeofoutputbuffer, refptr );
        }
        printf( "a.%s = \"%s\"\n", strs[i],  outputbuffer);
    }

    for( i=0; i < 10; ++i )
    {
        reference_create_int( key_ref, i );
        refptr = reference_get_item(htable, key_ref, 0 );
        neededspace = reference_extract_str(outputbuffer, sizeofoutputbuffer, refptr );
        if ( (neededspace + 1) > sizeofoutputbuffer )
        {
            sizeofoutputbuffer = neededspace * 2;
            outputbuffer = realloc ( outputbuffer, sizeofoutputbuffer);
            neededspace = reference_extract_str(outputbuffer, sizeofoutputbuffer, refptr );
        }
        printf( "a[%d] = \"%s\"\n", (int)i, outputbuffer );
    }

    mi = mallinfo();
    printf("inuse blocks: %d\n", mi.uordblks);

    for( i=0; i < num_strs; ++i )
    {
        reference_create_int( key_ref, i );
        reference_create_str( ref, strs[i] );
        reference_set_item( htable, key_ref, ref, 0 );
    }
    printf("Done strings by index.\n");

    /* reference_create_str( ref, "SESC" );
     */
    reference_create_int( ref, 4 );
    for( i=num_strs; i < 0xFFFFF; ++i )
    {
        reference_create_int( key_ref, i );
        reference_set_item( htable, key_ref, ref, 0 );
    }

    printf("Max Seed %d\n", (int)max_seed);
    printf("Done\n");
    mi = mallinfo();
    printf("inuse blocks: %d\n", mi.uordblks);
    for( i=0; i < 0xFF; ++i )
    {
        reference_create_int( key_ref, i );
        refptr = reference_get_item(htable, key_ref, 0 );
        neededspace = reference_extract_str(outputbuffer, sizeofoutputbuffer, refptr );
        if ( (neededspace + 1) > sizeofoutputbuffer )
        {
            sizeofoutputbuffer = neededspace * 2;
            outputbuffer = realloc ( outputbuffer, sizeofoutputbuffer);
            neededspace = reference_extract_str(outputbuffer, sizeofoutputbuffer, refptr );
        }
        printf( "a[%d] = \"%s\"\n", (int)i, outputbuffer );
    }

    reference_clear( htable );
    mi = mallinfo();
    printf("inuse blocks: %d\n", mi.uordblks);

    reference_clear( ref );
    reference_clear( key_ref );
    free( outputbuffer);
    mi = mallinfo();
    printf("inuse blocks: %d\n", mi.uordblks);

}

#endif /* HASHTEST */







