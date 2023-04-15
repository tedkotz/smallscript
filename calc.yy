%{
#include<stdio.h>
#include<stdlib.h>
#include"calc.h"
#include "y.tab.h"

double regs[26];
double base;

%}

%start list

%union { double a; int b; }


%token DIGIT LETTER EXIT

%left '|'
%left '&'
%left '+' '-'
%left '*' '/' '%'
%left UMINUS  /*supplies precedence for unary minus */

%type <b> LETTER DIGIT
%type <a> expr number


%%                   /* beginning of rules section */

list:                       /*empty */
         |
        list stat ';'
         |
        list EXIT
         {
           exit(0);
         }
         |
        list error ';'
         {
           yyerrok;
         }
         ;

stat:    expr
         {
           printf("%f\n",$1);
         }
         ;

expr:    '(' expr ')'
         {
           $$ = $2;
         }
         |
         LETTER '=' expr
         {
           $$ = regs[$1] = $3;
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
           $$ = (int)$1 % (int)$3;
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
           $$ = (int)$1 & (int)$3;
         }
         |
         expr '|' expr
         {
           $$ = (int)$1 | (int)$3;
         }
         |

        '-' expr %prec UMINUS
         {
           $$ = -$2;
         }
         |
         LETTER
         {
           $$ = regs[$1];
         }

         |
         number
         ;

number:  DIGIT
         {
           $$ = (double)$1;
           base = ($1==0) ? 8.0 : 10.0;
         }
         |
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
