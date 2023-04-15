%{

#include <stdio.h>
#include "y.tab.h"
#include "calc.h"
int c;
%}
%%
[\0- ]       ; // ignore whitespace

exit      {
            return(EXIT);
          }
[a-z][a-zA-Z0-9]* {
            c = yytext[0];
            yylval.b = c - 'a';
            return(LETTER);
          }
_[a-z]    {
            c = yytext[1];
            yylval.b = c - 'a';
            return(LETTER);
          }
[A-Z]+     {
            c = yytext[0];
            yylval.b = c - 'A';
            return(LETTER);
          }
[0-9]     {
            c = yytext[0];
            yylval.b = c - '0';
            return(DIGIT);
          }
[^a-zA-Z0-9\b]    {
                 c = yytext[0];
                 return(c);
              }
%%
