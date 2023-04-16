%{
#include<stdio.h>
#include"sesc_lex.h"
#include"hashtable.h"
#include"smallscript.h"
#include <malloc.h>


void yyerror(char * s);
int yywrap(void);

sesc_context *ctx = NULL;
intptr_t sizeofbuffer = 2;
char * buffer = NULL;
struct mallinfo mi;

%}

%start list

%union { intptr_t ref[2]; }


%token SYMBOL NUMBER STRING BOOL NONE BYTES FOR WHILE IN IF ELSE ELIF AND OR
%token NOT XOR EXIT PRINT PRINTLN LE EQ NE GE FLOORDIV POWER LSHIFT RSHIFT

%right '='
%left '|'
%left '&'
%left '+' '-'
%left '*' '/' '%'
%left UMINUS  /*supplies precedence for unary minus */

%type <ref> SYMBOL NUMBER STRING BYTES BOOL NONE expr


%%                   /* beginning of rules section */

list:                       /*empty */
         |
        list stat '\n'
         |
        list EXIT
         {
           return 0;
         }
         |
        list error '\n'
         {
           yyerrok;
         }
         ;

stat:    expr
         {
            intptr_t size = reference_extract_str(buffer, sizeofbuffer, $1 );
            if ( (size + 2) > sizeofbuffer )
            {
                sizeofbuffer = size * 2;
                buffer = realloc ( buffer, sizeofbuffer);
                size = reference_extract_str(buffer, sizeofbuffer, $1 );
            }
            buffer[size++]='\n';
            buffer[size++]='\0';
            fputs( buffer, stdout );
            reference_clear($1);
            mi = mallinfo();
            printf("inuse blocks: %d\n", mi.uordblks);

         }
         ;

expr:    '(' expr ')'
         {
           reference_init($$);
           reference_move( $$, $2);
         }
         |
         SYMBOL '=' expr
         {
           reference_init($$);
           reference_set_item(ctx, $1, $3, 0 );
           reference_clear($1);
           reference_move( $$, $3);
         }
         |
         expr '*' expr
         {
           reference_init($$);
           reference_create_int( $$, reference_extract_int($1)*reference_extract_int($3));
           reference_clear($1);
           reference_clear($3);
         }
         |
         expr '/' expr
         {
           reference_init($$);
           reference_create_int( $$, reference_extract_int($1)/reference_extract_int($3));
           reference_clear($1);
           reference_clear($3);
         }
         |
         expr '%' expr
         {
           reference_init($$);
           reference_create_int( $$, reference_extract_int($1)%reference_extract_int($3));
           reference_clear($1);
           reference_clear($3);
         }
         |
         expr '+' expr
         {
           reference_init($$);
           reference_create_int( $$, reference_extract_int($1)+reference_extract_int($3));
           reference_clear($1);
           reference_clear($3);
         }
         |
         expr '-' expr
         {
           reference_init($$);
           reference_create_int( $$, reference_extract_int($1)-reference_extract_int($3));
           reference_clear($1);
           reference_clear($3);
         }
         |
         expr '&' expr
         {
           reference_init($$);
           reference_create_int( $$, reference_extract_int($1)&reference_extract_int($3));
           reference_clear($1);
           reference_clear($3);
         }
         |
         expr '|' expr
         {
             reference_init($$);
             reference_create_int( $$, reference_extract_int($1)|reference_extract_int($3));
             reference_clear($1);
             reference_clear($3);
         }
         |

        '-' expr %prec UMINUS
         {
             reference_init($$);
             reference_create_int( $$, -reference_extract_int($2));
             reference_clear($2);
         }
         |
         SYMBOL
         {
             reference_init($$);
             reference_copy($$ , reference_get_item(ctx, $1, 0 ));
             reference_clear( $1 );
         }

         | NUMBER
         | BOOL
         | NONE
         | BYTES
         | STRING
         ;

%%
int main(unused int argc, unused char** argv)
{
    sesc_attr *attr=NULL;

    mi = mallinfo();
    printf("inuse blocks: %d\n", mi.uordblks);

    buffer = malloc( sizeofbuffer );

    attr = sesc_attr_create();
    ctx = sesc_context_create(attr);
    sesc_attr_destroy(attr);

    yyparse();

    sesc_context_destroy(ctx);

    mi = mallinfo();
    printf("inuse blocks: %d\n", mi.uordblks);

    return 0;
}

void yyerror(char * s)
{
  fprintf(stderr, "%s\n",s);
}

int yywrap(void)
{
  return(1);
}
