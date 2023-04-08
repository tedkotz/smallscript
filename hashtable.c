#include "smallc.h"


#define SESC_TYPE_NONE          0
#define SESC_TYPE_STR           2
#define SESC_TYPE_STATIC_STR    0x12
#define SESC_TYPE_SUBTABLE      0x11
#define SESC_TYPE intptr_t

#define refable_ptr           intptr_t*
#define refable_refcnt_idx    0
#define refable_data_idx      1
#define refable_header_size   (refable_data_idx*SIZEOFINT)

#define refable_str_len_idx   refable_data_idx
#define refable_str_bytes_idx (refable_data_idx+1)


#define reference_ptr       intptr_t*
#define ref_val_head        0
#define ref_val_data        1
#define reference_len       2
#define reference_size     (reference_len*SIZEOFINT)

// struct ht_item {
//     intptr_t key_ptr;
//     intptr_t key_len;
//     intptr_t value_header;
//     intptr_t value_data;
// };
#define ht_item_ptr intptr_t*
#define ht_item_len 4
#define ht_item_size (ht_item_len*SIZEOFINT)
#define ht_item_key_ptr  0
#define ht_item_key_len  1
#define ht_item_val_head 2
#define ht_item_val_data 3

// typedef htitem[256] htable
#define htable_ptr intptr_t*
#define htable_len 256
#define htable_size (htable_len * ht_item_size)

const intptr_t REFERENCE_NONE[reference_len] = { SESC_TYPE_NONE, 0 };


unsigned char hash_bytes( const char* data, intptr_t len, char seed )
{
    //input seeds will probably be sequential so spin them once
    intptr_t sum  =  (0xfff1 * ((intptr_t)seed+1));
    while( len-- > 0 )
    {
        intptr_t tmp = *data++;
        // might be faster without native multiply sum = ((sum << 1) + sum) ^ ((tmp << 5) - tmp);
        sum = (3 * sum) ^ (31 * tmp);
    }
    return sum;
}

bool reference_countable_type( SESC_TYPE type )
{
    switch (type)
    {
        case SESC_TYPE_STR:
            return true;

    }
    return false;
}

void refable_header_init( refable_ptr data )
{
   data[refable_refcnt_idx]=0;
}

void reference_init( reference_ptr ref )
{
    ref[ref_val_head]=SESC_TYPE_NONE;
    ref[ref_val_data]=0;
}

void reference_clear( reference_ptr ref )
{
    if(reference_countable_type(ref[ref_val_head]))
    {
        refable_ptr tmp = (refable_ptr)ref[ref_val_data];
        --(tmp[refable_refcnt_idx]);
        if(tmp[refable_refcnt_idx] < 1)
        {
            free(tmp);
        }
    }
    ref[ref_val_head]=SESC_TYPE_NONE;
    ref[ref_val_data]=0;
}

void reference_set_refable( reference_ptr dst_ref, SESC_TYPE val_head, refable_ptr val_data  )
{
    if( dst_ref[ref_val_head]!=val_head ||
        dst_ref[ref_val_data]!=(intptr_t)val_data )
    {
        if(reference_countable_type(val_head))
        {
            ++(val_data[refable_refcnt_idx]);
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

void reference_create_str( reference_ptr ref, const char * str )
{
    intptr_t len = strlen(str)+1;
    refable_ptr data= malloc(refable_header_size+SIZEOFINT+len);
    if( data != NULL )
    {
        refable_header_init( data );
        data[refable_str_len_idx] = len;
        strncpy ( (char*)&data[refable_str_bytes_idx], str, len);
        reference_set_refable(  ref, SESC_TYPE_STR, data );
    }
    else
    {
        reference_clear(ref);
    }
}

const char * reference_extract_str( const reference_ptr ref )
{
    if( SESC_TYPE_STR != ref[ref_val_head])
    {
        return "";
    }
    else
    {
        refable_ptr data=(refable_ptr)ref[ref_val_data];
        return (char*)&data[refable_str_bytes_idx];
    }
}



htable_ptr htable_create(void)
{
    return calloc( htable_len, ht_item_size);
}

void htable_set_item( htable_ptr table, const char* key_ptr, intptr_t key_len, const reference_ptr src_ref, char seed)
{
    unsigned hash = hash_bytes(key_ptr, key_len, seed);
    ht_item_ptr entry = table + (ht_item_len * hash);

    if (entry[ht_item_val_head] == SESC_TYPE_NONE)
    {
        // empty
        if( src_ref[ref_val_head] != SESC_TYPE_NONE )
        {
            entry[ht_item_key_len] = key_len;
            reference_copy( &entry[ht_item_val_head], src_ref);

            char * dst = malloc(key_len);
            entry[ht_item_key_ptr] = (intptr_t)dst;
            const char * src = key_ptr;
            while (key_len-- > 0)
            {
                *dst++ = *src++;
            }
        }
    }
    else if (entry[ht_item_val_head] == SESC_TYPE_SUBTABLE)
    {
        // check subtable
        return htable_set_item( (htable_ptr)entry[ht_item_val_data], key_ptr, key_len, src_ref, seed+1);
    }
    else if (key_len == entry[ht_item_key_len] && (0 == memcmp((char*)entry[ht_item_key_ptr], key_ptr, key_len)))
    {
        // Replace
        reference_copy( &entry[ht_item_val_head], src_ref);
    }
    else
    {
        // Collision

        htable_ptr tmp = htable_create();

        htable_set_item( tmp, (char*)entry[ht_item_key_ptr], entry[ht_item_key_len], &entry[ht_item_val_head], seed+1);

        reference_clear( &entry[ht_item_val_head] );
        free( (char*)entry[ht_item_key_ptr] );

        entry[ht_item_val_data] = (intptr_t)tmp;
        entry[ht_item_val_head] = SESC_TYPE_SUBTABLE;

        return htable_set_item( (htable_ptr)entry[ht_item_val_data], key_ptr, key_len, src_ref, seed+1);
    }
}

const reference_ptr htable_get_item ( htable_ptr table, const char* key_ptr, intptr_t key_len, char seed)
{
    unsigned hash = hash_bytes(key_ptr, key_len, seed);
    ht_item_ptr entry = table + (ht_item_len * hash);

    if (entry[ht_item_val_head] == SESC_TYPE_SUBTABLE)
    {
        // check subtable
        return htable_get_item( (htable_ptr)entry[ht_item_val_data], key_ptr, key_len, seed+1);
    }
    else if (key_len == entry[ht_item_key_len] && (0 == memcmp((char*)entry[ht_item_key_ptr], key_ptr, key_len)))
    {
        // Replace
        return &entry[ht_item_val_head];
    }
    else
    {
        return REFERENCE_NONE;
    }
}


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

    const int num_strs = sizeof(strs)/sizeof(strs[0]);
    //reference_ptr refptr;
    intptr_t ref[reference_len];
    reference_init(ref);



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

    htable_ptr htable = htable_create();

    for( int i=3; i < num_strs; ++i )
    {
        int len = strlen(strs[i]);
        reference_create_str( ref, strs[i-3] );
        htable_set_item( htable, strs[i], len, ref, 0 );
        reference_clear( ref );
    }

    for( int i=3; i < num_strs; ++i )
    {
        int len = strlen(strs[i]);
        printf( "a.%s = \"%s\"\n", strs[i], reference_extract_str( htable_get_item(htable, strs[i], len, 0 ) ) );
    }



}


// ADD REFERNCE COUNTING TO HASHTABLE









