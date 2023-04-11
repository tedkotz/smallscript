#include "smallc.h"
#include "smallscript.h"

int main( int argc, intptr_t* argv )
{
    fputs("Hello World\n", stderr);
    if(argc >= 2)
    {
        fputs((char*)argv[1], stderr);
    }
    sesc_attr *attr=NULL;
    sesc_context *ctx = NULL;
    attr = sesc_attr_create();
    ctx = sesc_context_create(attr);
    sesc_eval_string(ctx, "b=add(1,2);");
    printf("1+2=%d\n", sesc_get_int_by_idx(ctx, -1));
    sesc_eval_string(ctx, "a=sub(2,5);");
    printf("2-5=%d\n", sesc_get_int_by_name(ctx, "a"));
    printf("b=%d\n", sesc_get_int_by_name(ctx, "b"));
    printf("add=%d\n", sesc_get_int_by_name(ctx, "add"));

    //REPL
    //char line[128];
    //while(sesc_context_active(ctx))
    //{
    //    fgets(line, 128, stdin);
    //    sesc_eval_string(ctx, line);
    //    //fputs( sesc_get_string( ctx, OUTPUT_BUFFER );
    //}

    sesc_context_destroy(ctx);
    sesc_attr_destroy(attr);
    return 0;
}
