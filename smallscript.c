#include "smallc.h"
#include "smallscript.h"


typedef enum SESC_TYPE
{
//a string
//a number
//an object (JSON object)
//an array
//a boolean
//null
    Type_none, // is this just an empty object?
    //Type_null,
    //Type_nil,
    //??undefined
    Type_bool, // is this just an int
    Type_list,
    //Type_vector
    //Type_array,
    //Type_tuple,
    //Type_range,
    Type_object, //(JSON object),table, dict or map
    Type_int,
    //Type_byte,
    //Type_number
    //Type_float,
    //Type_complex,
    Type_str,
    Type_bytes,
    //Type_BigInt,
    //Type_BigNumber,
    //Type_userdata,
    Type_function,

    //??Type thread
    //??Date // numeric type??

} SESC_TYPE;

typedef int* sesc_Object;

#define REFERENCE_HEADER 0
#define REFERENCE_DATA 1

#define REFERENCE_HEADER_IMMUTABLE 0x0001

#define REFERENCE_NONE_HEADER      0x0001
#define REFERENCE_NONE_DATA        0x0001

#define REFERENCE_BOOL_HEADER      0x0001
#define REFERENCE_FALSE_DATA       0x0000


#define REFERENCE_HEADER_NONE      0x0001
#define REFERENCE_HEADER_ORDINAL   0x0001


typedef struct reference
{
    intptr_t head;
    intptr_t data;
} reference;

struct sesc_attr
{
    int version;
};

struct sesc_context
{
    int* stack;
    sesc_Object scope;
};

sesc_attr* sesc_attr_create(void)
{
    return NULL;
}

sesc_context* sesc_context_create(sesc_attr* attr)
{
    return NULL;
}

int sesc_eval_string(sesc_context* ctx, const char * str)
{
    return 0;
}

int sesc_get_int(sesc_context* ctx, int id)
{
    return 0;
}

const char* sesc_get_string(sesc_context* ctx, int id)
{
    return "";
}

void sesc_context_destroy(sesc_context* ctx)
{
}

void sesc_attr_destroy(sesc_attr* ctx)
{
}




