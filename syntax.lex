%{

#include <stdio.h>
#include "y.tab.h"
#include "syntax.h"
int c;
%}
%%
[\0- ]       ; //ignore whitespace
\"(\\.|[^"\\])*\" {
            // String literal
            c = yytext[0];
            yylval.a = c - 'a';
            return(STRING);
          }
\'(\\.|[^'\\])*\' {
            // String literal
            c = yytext[0];
            yylval.a = c - 'a';
            return(STRING);
          }
for  {
            return(FOR);
          }
while  {
            return(WHILE);
          }
in  {
            return(IN);
          }
if  {
            return(IF);
          }
else  {
            return(ELSE);
          }
elif  {
            return(ELIF);
          }
False  {
            return(FALSE);
          }
True  {
            return(TRUE);
          }
None  {
            return(NONE);
          }
and  {
            return(AND);
          }
or  {
            return(OR);
          }
[a-zA-Z_][a-zA-Z0-9_] {
            // Symbol name
            c = yytext[0];
            yylval.a = c - 'a';
            return(SYMBOL);
          }
0x[0-9a-zA-Z][0-9a-zA-Z]*  {
            c = yytext[0];
            yylval.a = c - '0';
            return(NUMBER);
          }
[0-9]+  {
            c = yytext[0];
            yylval.a = c - '0';
            return(NUMBER);
          }
[^a-zA-Z0-9\b]    {
                 c = yytext[0];
                 return(c);
              }
%%
