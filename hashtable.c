#include "smallc.h"


char hash_bytes( char* data, intptr_t len, char seed )
{
    while( len-- > 0 )
    {
        seed = seed << 2 + seed + *data++;
    }
    return seed;
}

struct hash_item {
    intptr_t key_ptr;
    intptr_t key_len;
    intptr_t value_header;
    intptr_t value_data;
};

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

void htable_init( htable_ptr table )
{
    intptr_t len = htable_len * ht_item_len;
    while (len-- > 0)
    {
        *table++ = 0;
    }
}

void htable_add_item( htable_ptr table, intptr_t key_ptr, intptr_t key_len, intptr_t value_header, intptr_t value_data)
{
    char hash = hash_bytes(key_ptr, key_len, 0);
    ht_item_ptr entry = table + (ht_item_len * hash);

    if (entry[ht_item_val_head] == 0)
    {
        // empty
        entry[ht_item_key_len] = key_len;
        entry[ht_item_val_head] = value_header;
        entry[ht_item_val_data] = value_data;

        char * dst = malloc(key_len);
        entry[ht_item_key_len] = ptr;
        char * src = key_ptr;
        while (key_len-- > 0)
        {
            *ptr++ = *src++;
        }
    }
    else if (key_len == entry[ht_item_val_head] && memcmp(entry[ht_item_key_ptr], key_ptr, key_len))
    {
        // Replace
        entry[ht_item_val_head] = value_header;
        entry[ht_item_val_data] = value_data;
    }
    else
    {
        // Collision
    }
}











