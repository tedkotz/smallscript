%{

#include <stdio.h>
#include "y.tab.h"
#include "calc.h"
#include "hashtable.h"

%}
%%
[\0- ]       ; // ignore whitespace

exit      {
            return(EXIT);
          }

[a-zA-Z_][a-zA-Z0-9_]* {
            // Symbol name
            reference_init( yylval.ref );
            reference_create_str( yylval.ref, yytext );
            return(SYMBOL);
          }

0x[0-9a-zA-Z][0-9a-zA-Z]*  {
            reference_init( yylval.ref );
            reference_create_int( yylval.ref, strtoll( yytext, NULL, 0) );
            return(NUMBER);
          }

[0-9]+   {
            reference_init( yylval.ref );
            reference_create_int( yylval.ref, strtoll( yytext, NULL, 0) );
            return(NUMBER);
          }

[^a-zA-Z0-9\b]    {
                 return(yytext[0]);
              }
%%
