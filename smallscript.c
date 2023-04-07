#include "smallc.h"
#include "smallscript.h"


typedef enum sscript_Types
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

} sscript_Types;

typedef int* sscript_Object;

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
    intptr_t
} reference;

struct sscript_attr
{
    int version;
};

struct sscript_context
{
    int* stack;
    sscript_Object scope;
};

sscript_attr* sscript_attr_create(void)
{
    return NULL;
}

sscript_context* sscript_context_create(sscript_attr* attr)
{
    return NULL;
}

int sscript_eval_string(sscript_context* ctx, const char * str)
{
    return 0;
}

int sscript_get_int(sscript_context* ctx, int id)
{
    return 0;
}

const char* sscript_get_string(sscript_context* ctx, int id)
{
    return "";
}

void sscript_context_destroy(sscript_context* ctx)
{
}

void sscript_attr_destroy(sscript_attr* ctx)
{
}




