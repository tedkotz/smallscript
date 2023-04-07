#include "smallc.h"
#include "smallscript.h"

int main( int argc, intptr_t* argv )
{
    fputs("Hello World\n", stderr);
    if(argc >= 2)
    {
        fputs((char*)argv[1], stderr);
    }
    sscript_attr *attr=NULL;
    sscript_context *ctx = NULL;
    attr = sscript_attr_create();
    ctx = sscript_context_create(attr);
    sscript_eval_string(ctx, "1+2");
    printf("1+2=%d\n", sscript_get_int(ctx, -1));

    //REPL
    //char line[128];
    //while(sscript_context_active(ctx))
    //{
    //    fgets(line, 128, stdin);
    //    sscript_eval_string(ctx, line);
    //    //fputs( sscript_get_string( ctx, OUTPUT_BUFFER );
    //}

    sscript_context_destroy(ctx);
    sscript_attr_destroy(attr);
    return 0;
}
