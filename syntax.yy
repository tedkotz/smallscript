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

%start body

%union { intptr_t ref[reference_len]; }


%token SYMBOL NUMBER STRING BOOL NONE BYTES FOR WHILE IN IF ELSE ELIF AND OR
%token NOT XOR EXIT PRINT PRINTLN LE EQ NE GE FLOORDIV POWER LSHIFT RSHIFT

%right '='                                  /* Assignment */
%left OR                                    /* Logical sums */
%left AND                                   /* Logical products */
%left NOT                                   /* Logical unaries */
%left LE EQ NE GE '<' '>'                   /* Arithmetic to Logical comparators*/
%left '+' '-' '^' '|'                       /* Arithmetic sums */
%left '*' '/' '%' FLOORDIV '&'              /* Arithmetic products */
%left POWER LSHIFT RSHIFT                   /* Arithmetic exponentials */
%left UNARY                                 /* Arithmetic unaries */
%left '.'                                   /* Binding */

%type <ref> NUMBER SYMBOL STRING BYTES BOOL NONE expr

/* list object function bool none
 */


%%                   /* beginning of rules section */

body:                       /*empty */
         |
        body stat ';'
         |
        body error ';'
         {
           yyerrok;
         }
         ;

stat:    expr
         {
            /* free the expression. */
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
         |
         EXIT
         {
             return 0;
         }
         ;
/*
 *         |
 *         FOR SYMBOL IN list '{' body '}'
 *         |
 *         WHILE expr '{' body '}'
 *         |
 *         IF bool '{' body '}' ellist ELSE '{' body '}'
 *         |
 *         IF bool '{' body '}' ELSE '{' body '}'
 *         |
 *         IF bool '{' body '}' ellist
 *         |
 *         IF bool '{' body '}'
 *
 * ellist:  ellist ELIF bool '{' body '}'
 *          |
 *          ELIF bool '{' body '}'
 *          ;
 *
 * lvalue:  SYMBOL
 *          |
 *          lvalue '.' SYMBOL
 *          |
 *          expr list
 *          ;
 *
 * bool:    lvalue
 *          |
 *          bool OR bool
 *          |
 *          bool AND bool
 *          |
 *          expr '<' expr
 *          |
 *          expr LE expr
 *          |
 *          expr EQ expr
 *          |
 *          expr NE expr
 *          |
 *          expr '>' expr
 *          |
 *          expr GE expr
 *          |
 *          TRUE
 *          |
 *          FALSE
 *          ;
 *
 * list:    '[' listbody ']'
 *          |
 *          '['']'
 *          |
 *          '[' expr ':' expr ':' expr ']'
 *          |
 *          '[' expr ':' expr ']'
 *          |
 *          '[' expr ']'
 *          |
 *          lvalue
 *          ;
 *
 * listbody: listbody ',' expr
 *          ;
 */

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

        '-' expr %prec UNARY
         {
             reference_init($$);
             reference_create_int( $$, -reference_extract_int($2));
             reference_clear($2);
         }
         |
        '+' expr %prec UNARY
         {
             reference_init($$);
             reference_move($$, $2);
         }
         |
        '~' expr %prec UNARY
         {
             reference_init($$);
             reference_create_int( $$, ~reference_extract_int($2));
             reference_clear($2);
         }

         |
         SYMBOL
         {
             /* lvalue */
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
