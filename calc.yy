%{
#include<stdio.h>
#include"sesc_lex.h"
#include"hashtable.h"
#include"smallscript.h"


void yyerror(char * s);
int yywrap(void);

sesc_context *ctx = NULL;
intptr_t sizeofbuffer = 2;
char * buffer = NULL;

%}

%start list

%union { intptr_t ref[2]; }


%token SYMBOL NUMBER STRING BOOL NONE BYTES FOR WHILE IN IF ELSE ELIF AND OR NOT XOR EXIT PRINT PRINTLN

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
            // free the expression.
            reference_clear($1);
         }
         |
         PRINT '[' expr ']'
         {
            intptr_t size = reference_extract_str(buffer, sizeofbuffer, $3 );
            if ( (size + 2) > sizeofbuffer )
            {
                sizeofbuffer = size * 2;
                buffer = realloc ( buffer, sizeofbuffer);
                size = reference_extract_str(buffer, sizeofbuffer, $3 );
            }
            fputs( buffer, stdout );
            reference_clear($3);
         }
         |
         PRINTLN '[' expr ']'
         {
            intptr_t size = reference_extract_str(buffer, sizeofbuffer, $3 );
            if ( (size + 2) > sizeofbuffer )
            {
                sizeofbuffer = size * 2;
                buffer = realloc ( buffer, sizeofbuffer);
                size = reference_extract_str(buffer, sizeofbuffer, $3 );
            }
            buffer[size++]='\n';
            buffer[size++]='\0';
            fputs( buffer, stdout );
            reference_clear($3);
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
int main(int argc, char** argv)
{
    sesc_attr *attr=NULL;

    buffer = malloc( sizeofbuffer );

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
