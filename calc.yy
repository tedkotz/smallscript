%{
#include<stdio.h>
#include"calc.h"
#include"hashtable.h"
#include"smallscript.h"
#include "y.tab.h"

sesc_context *ctx = NULL;

%}

%start list

%union { intptr_t ref[2]; }


%token NUMBER SYMBOL EXIT

%left '|'
%left '&'
%left '+' '-'
%left '*' '/' '%'
%left UMINUS  /*supplies precedence for unary minus */

%type <ref> SYMBOL NUMBER expr


%%                   /* beginning of rules section */

list:                       /*empty */
         |
        list stat ';'
         |
        list EXIT
         {
           return 0;
         }
         |
        list error ';'
         {
           yyerrok;
         }
         ;

stat:    expr
         {
           printf("%s\n", reference_extract_str($1));
           reference_clear($1);
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

         |
         NUMBER
         ;

%%
int main(int argc, char** argv)
{
    sesc_attr *attr=NULL;
    attr = sesc_attr_create();
    ctx = sesc_context_create(attr);
    sesc_attr_destroy(attr);

    yyparse();

    sesc_context_destroy(ctx);

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
