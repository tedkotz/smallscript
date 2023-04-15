%{
#include<stdio.h>
#include"syntax.h"
#include "y.tab.h"

int regs[26];
int base;

%}

%start body

%union { intptr_t a[reference_len]; }


%token NUMBER SYMBOL STRING FOR IN WHILE IF ELIF ELSE

%left '|'
%left '&'
%left '+' '-'
%left '*' '/' '%'
%left '.'
%left 'OR'
%left 'AND'
%left UNARY  /*supplies precedence for unary minus */

%type <a> NUMBER SYMBOL STRING stat expr list object function bool none


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
           printf("%d\n",$1);
         }
         |
         FOR SYMBOL IN list '{' body '}'
         |
         WHILE expr '{' body '}'
         |
         IF bool '{' body '}' ellist ELSE '{' body '}'
         |
         IF bool '{' body '}' ELSE '{' body '}'
         |
         IF bool '{' body '}' ellist
         |
         IF bool '{' body '}'
         ;

ellist:  ellist ELIF bool '{' body '}'
         |
         ELIF bool '{' body '}'
         ;

lvalue:  SYMBOL
         |
         lvalue '.' SYMBOL
         |
         expr list
         ;

bool:    lvalue
         |
         bool OR bool
         |
         bool AND bool
         |
         expr '<' expr
         |
         expr '<=' expr
         |
         expr '==' expr
         |
         expr '!=' expr
         |
         expr '>' expr
         |
         expr '>=' expr
         |
         TRUE
         |
         FALSE
         ;

list:    '[' listbody ']'
         |
         "[]"
         |
         '[' expr ':' expr ':' expr ']'
         |
         '[' expr ':' expr ']'
         |
         '[' expr ']'
         |
         lvalue
         ;

listbody: listbody ',' expr
         ;


expr:    '(' expr ')'
         {
           $$ = $2;
         }
         |
         lvalue '=' expr
         {
           regs[$1] = $3;
         }
         |
         expr '*' expr
         {

           $$ = $1 * $3;
         }
         |
         expr '/' expr
         {
           $$ = $1 / $3;
         }
         |
         expr '%' expr
         {
           $$ = $1 % $3;
         }
         |
         expr '+' expr
         {
           $$ = $1 + $3;
         }
         |
         expr '-' expr
         {
           $$ = $1 - $3;
         }
         |
         expr '&' expr
         {
           $$ = $1 & $3;
         }
         |
         expr '|' expr
         {
           $$ = $1 | $3;
         }
         |

        '-' expr %prec UNARY
         {
           $$ = -$2;
         }
         |
        '+' expr %prec UNARY
         {
           $$ = $2;
         }
         |
        '!' expr %prec UNARY
         {
           $$ = -$2;
         }
         |
        '~' expr %prec UNARY
         {
           $$ = -$2;
         }
         |
         lvalue
         {
           $$ = regs[$1];
         }
         |
         NUMBER
         ;

number:  DIGIT
         {
           $$ = $1;
           base = ($1==0) ? 8 : 10;
         }       |
         number DIGIT
         {
           $$ = base * $1 + $2;
         }
         ;

%%
int main(int argc, char** argv)
{
 return(yyparse());
}

void yyerror(char * s)
{
  fprintf(stderr, "%s\n",s);
}

int yywrap(void)
{
  return(1);
}
