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
    //printf("1+2=%"PRIdPTR"\n", sesc_get_int_by_idx(ctx, -1));
    sesc_eval_string(ctx, "print('1+2:',1);");
    sesc_eval_string(ctx, "print(b,0);");
    sesc_eval_string(ctx, "a=sub(2,5);");
    //printf("2-5=%"PRIdPTR"\n", sesc_get_int_by_name(ctx, "a"));
    sesc_eval_string(ctx, "print('2-5:',1);");
    sesc_eval_string(ctx, "print(a,0);");
    //printf("b=%"PRIdPTR"\n", sesc_get_int_by_name(ctx, "b"));
    sesc_eval_string(ctx, "print('b:',1);");
    sesc_eval_string(ctx, "print(b,0);");
    sesc_eval_string(ctx, "print(132,0);");
    printf("add=%"PRIdPTR"\n", sesc_get_int_by_name(ctx, "add"));
    sesc_eval_string(ctx, "print('What is your name?',0);");
    sesc_eval_string(ctx, "name=input();");
    sesc_eval_string(ctx, "print('Hello ',1);");
    sesc_eval_string(ctx, "print(name,1);");
    sesc_eval_string(ctx, "print(\"!!!\",0);");

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
