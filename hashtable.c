

/* Includes ******************************************************************/
#include "hashtable.h"

/* Defines *******************************************************************/
#define NONE_HASH 0

// enum SESC_TYPE
#define SESC_TYPE_NONE          0
#define SESC_TYPE_BOOL          1
#define SESC_TYPE_INT           2
#define SESC_TYPE_STR           3
#define SESC_TYPE_BYTES         4
#define SESC_TYPE_OBJ           5
#define SESC_TYPE_LIST          6
#define SESC_TYPE_FUNC          7

        // case SESC_TYPE_NONE  :
        // case SESC_TYPE_BOOL  :
        // case SESC_TYPE_INT   :
        // case SESC_TYPE_STR   :
        // case SESC_TYPE_BYTES :
        // case SESC_TYPE_OBJ   :
        // case SESC_TYPE_LIST  :
        // case SESC_TYPE_FUNC  :

#define SESC_TYPE_STATIC_STR    0x12
#define SESC_TYPE_SUBTABLE      0x11
#define SESC_TYPE intptr_t


/* Types *********************************************************************/
// struct refable
#define refable_ptr           intptr_t*
#define refable_refcnt_idx    0
#define refable_data_idx      1
#define refable_header_size   (refable_data_idx*SIZEOFINT)

// struct refable_str or refable_bytes {
//     intptr_t refcnt
//     intptr_t len
//     char     data[]
// }
#define refable_bytes_len_idx       refable_data_idx
#define refable_bytes_data_idx      (refable_data_idx+1)
#define refable_bytes_header_size   (refable_bytes_data_idx*SIZEOFINT)

// struct sesc_reference {
//     intptr_t header
//     intptr_t dataptr
// }
#define reference_ptr       intptr_t*
#define ref_val_head        0
#define ref_val_data        1
#define reference_len       2
#define reference_size     (reference_len*SIZEOFINT)

// struct ht_item {
//     sesc_reference key
//     sesc_reference value
// }
#define ht_item_ptr intptr_t*
#define ht_item_len 4
#define ht_item_size (ht_item_len*SIZEOFINT)
#define ht_item_key_head  0
#define ht_item_key_data  1
#define ht_item_val_head  2
#define ht_item_val_data  3


// struct refable_list or refable_object {
//     intptr_t refcnt
//     ht_item  table[htable_len]
// }
#define htable_len 256
#define refable_htable_data_idx  refable_data_idx
#define refable_htable_idx_cnt   (refable_data_idx+(htable_len*ht_item_len))
#define refable_htable_size      (refable_htable_idx_cnt*SIZEOFINT)

/* Interfaces ****************************************************************/

/* Data **********************************************************************/
#define REFERENCE_INIT { SESC_TYPE_NONE, 0 }
const intptr_t REFERENCE_NONE[] = REFERENCE_INIT;


intptr_t max_seed = 0;


/* Functions *****************************************************************/


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
        // case SESC_TYPE_FUNC  :
        // {
        //     refable_ptr tmp = (refable_ptr)ref[ref_val_data];
        //     --(tmp[refable_refcnt_idx]);
        //     if(tmp[refable_refcnt_idx] < 1)
        //     {
        //         //func_destroy(tmp);
        //     }
        //     break;
        // }

        case SESC_TYPE_STR   :
        case SESC_TYPE_BYTES :
        {
            //simple free
            refable_ptr tmp = (refable_ptr)ref[ref_val_data];
            --(tmp[refable_refcnt_idx]);
            if(tmp[refable_refcnt_idx] < 1)
            {
                free(tmp);
            }
            break;
        }
        // case SESC_TYPE_NONE  :
        // case SESC_TYPE_BOOL  :
        // case SESC_TYPE_INT   :
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
            // case SESC_TYPE_FUNC  :
                ++(val_data[refable_refcnt_idx]);
                break;

            // case SESC_TYPE_NONE  :
            // case SESC_TYPE_BOOL  :
            // case SESC_TYPE_INT   :
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

const char* reference_extract_str( const reference_ptr ref )
{
    switch (ref[ref_val_head])
    {
        case SESC_TYPE_NONE:
        {
            return "None";
        }

        case SESC_TYPE_BOOL  :
        {
            if(ref[ref_val_data])
            {
                return "True";
            }
            else
            {
                return "False";
            }
        }

        case SESC_TYPE_INT   :
        {
            return "int";
        }
        case SESC_TYPE_STR   :
        {
            refable_ptr data=(refable_ptr)ref[ref_val_data];
            return (char*)&data[refable_bytes_data_idx];
        }
        case SESC_TYPE_BYTES :
        {
            return "bytes";
        }
        case SESC_TYPE_OBJ   :
        {
            return "object";
        }
        case SESC_TYPE_LIST  :
        {
            return "list";
        }
        case SESC_TYPE_FUNC  :
        {
            return "func";
        }
    }
    fputs("reference_extract_str():Unknown Type", stderr);
    return "Unknown Type";
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
                reference_clear(dst_ref);

            case SESC_TYPE_BOOL  :
            case SESC_TYPE_INT   :
                // shallow copy works fine
                return reference_copy( dst_ref, src_ref );

            case SESC_TYPE_OBJ   :
            case SESC_TYPE_LIST  :
            case SESC_TYPE_FUNC  :
            {
                // TKOTZ - copy the objects someday?
                // shallow copy works fine?
                return reference_copy( dst_ref, src_ref );
            }

            case SESC_TYPE_STR   :
            case SESC_TYPE_BYTES :
            {
                // copy the data
                //refable_ptr data=(refable_ptr)ref[ref_val_data]
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

/** Hash Table Functions *****************************************************/

static unsigned char hash_bytes( const char* data, intptr_t len, char seed )
{
    // Need a good 8-bit hash value. Let's try the family of CRCs.
    // So let's use a 10-bit polynomial with the 8 seed bits being the middle terms
    // Note the terms here are bit 6 represents x^0 and
    // x^9 is assumed to be one by checking it before the shift
    // poly  15  14  13  12  11  10  09  08  07  06  05  04  03  02  01  00
    // seed      b7  b6  b5  b4  b3  b2  b1  b0
    // 1<<6                                      1
    // term      x^8 x^7 x^6 x^5 x^4 x^3 x^2 x^1 x^0
    intptr_t poly = (1 << 6) + (seed <<7) ;

    intptr_t accum = 0xFF00;
    char i;
    while( len-- > 0)
    {
        accum ^= (*data++ & 0x0FF);
        i=8;
        while(i-- > 0)
        {
            // Might be faster on sys without native multiply or branch prediction
            // if( accum & 0x8000 ) accum ^= poly;
            accum ^= (poly * ((accum >> 15) & 1));
            accum <<= 1;
        }
    }
    i=8;
    while(i-- > 0)
    {
        // Might be faster on sys without native multiply or branch prediction
        // if( accum & 0x8000 ) accum ^= poly;
        accum ^= (poly * ((accum >> 15) & 1));
        accum <<= 1;
    }
    return (accum >> 6) & 0x0FF;
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
        {
            // hash the reference
            return hash_bytes( (const char*)ref, reference_size, seed);
        }

        case SESC_TYPE_STR   :
        case SESC_TYPE_BYTES :
        {
            // hash the data
            refable_ptr data=(refable_ptr)ref[ref_val_data];
            return hash_bytes( (char*)&data[refable_bytes_data_idx], data[refable_bytes_len_idx], seed);
        }
    }
    fputs("reference_hash():Unknown Type", stderr);
    // hash the reference
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
            // intentional fall thru
        case SESC_TYPE_SUBTABLE:
            data=(refable_ptr)object_ref[ref_val_data];
            break;
        // case SESC_TYPE_NONE  :
        // case SESC_TYPE_BOOL  :
        // case SESC_TYPE_INT   :
        // case SESC_TYPE_STR   :
        // case SESC_TYPE_BYTES :
        // case SESC_TYPE_FUNC  :
        default:
        return;
    }
    unsigned hash = reference_hash(key_ref, seed);
    ht_item_ptr entry = data + (refable_htable_data_idx + ht_item_len * hash);

    if (entry[ht_item_val_head] == SESC_TYPE_NONE)
    {
        // Empty
        if( src_ref[ref_val_head] != SESC_TYPE_NONE )
        {
            reference_copy( &entry[ht_item_val_head], src_ref);
            reference_deep_copy( &entry[ht_item_key_head], key_ref);
        }
    }
    else if (entry[ht_item_val_head] == SESC_TYPE_SUBTABLE)
    {
        // check subtable
        return reference_set_item( &entry[ht_item_val_head], key_ref, src_ref, seed+1);
    }
    else if (0 == reference_cmp(key_ref, &entry[ht_item_key_head]))
    {
        // Replace
        reference_copy( &entry[ht_item_val_head], src_ref);
    }
    else if (seed+1 != 0)
    {
        // Collision
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


const reference_ptr reference_get_item( reference_ptr object_ref, const reference_ptr key_ref, char seed)
{
    refable_ptr data = NULL;
    switch (object_ref[ref_val_head])
    {
        case SESC_TYPE_OBJ   :
        case SESC_TYPE_LIST  :
            seed = 0;
            // intentional fall thru
        case SESC_TYPE_SUBTABLE:
            data=(refable_ptr)object_ref[ref_val_data];
            break;
        // case SESC_TYPE_NONE  :
        // case SESC_TYPE_BOOL  :
        // case SESC_TYPE_INT   :
        // case SESC_TYPE_STR   :
        // case SESC_TYPE_BYTES :
        // case SESC_TYPE_FUNC  :
        default:
        return REFERENCE_NONE;
    }
    unsigned hash = reference_hash(key_ref, seed);
    ht_item_ptr entry = data + (refable_htable_data_idx + ht_item_len * hash);

    if (entry[ht_item_val_head] == SESC_TYPE_SUBTABLE)
    {
        // Check subtable
        return reference_get_item( &entry[ht_item_val_head], key_ref, seed+1);
    }
    else if (0 == reference_cmp(key_ref, &entry[ht_item_key_head]))
    {
        // Found
        return &entry[ht_item_val_head];
    }
    else
    {
        return REFERENCE_NONE;
    }
}

// TODO - TKOTZ
// Needs Callable functionality
// Needs parser
// Needs way to detect circular references
//   scan contents of htable for itself?
//   prevent an htable from containing itselfas key or value?
// Needs iterators

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
    //reference_ptr refptr;
    intptr_t ref[] = REFERENCE_INIT;
    intptr_t key_ref[] = REFERENCE_INIT;
    intptr_t htable[]  = REFERENCE_INIT;


    for( int i=0; i < num_strs; ++i )
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
        printf( "a.%s = \"%s\"\n", strs[i], reference_extract_str( reference_get_item(htable, key_ref, 0 ) ) );
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
        printf( "a.%s = \"%s\"\n", strs[i], reference_extract_str( reference_get_item(htable, key_ref, 0 ) ) );
    }

    for( i=0; i < 10; ++i )
    {
        reference_create_int( key_ref, i );
        printf( "a[%d] = \"%s\"\n", (int)i, reference_extract_str( reference_get_item(htable, key_ref, 0 ) ) );
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

    reference_create_str( ref, "Ted" );
    for( i=num_strs; i < 0xFFFF; ++i )
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
        printf( "a[%d] = \"%s\"\n", (int)i, reference_extract_str( reference_get_item(htable, key_ref, 0 ) ) );
    }

    reference_clear( htable );
    mi = mallinfo();
    printf("inuse blocks: %d\n", mi.uordblks);

    reference_clear( ref );
    reference_clear( key_ref );
    mi = mallinfo();
    printf("inuse blocks: %d\n", mi.uordblks);
}

#endif // HASHTEST







