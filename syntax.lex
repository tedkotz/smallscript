%{



#include <stdio.h>
#include "hashtable.h"
#include "calc.h"
#ifdef CALC
#include "calc.tab.h"
#else
#include "syntax.tab.h"
#endif

char hexdigit( char c )
{
    if( c >= '0' &&c <= '9')
    {
        return c-'0';
    }
    else if( c >= 'a' &&c <= 'f')
    {
        return c-'a'+10;
    }
    else if( c >= 'A' &&c <= 'F')
    {
        return c-'A'+10;
    }
    return 0;
}


%}
%%
[\0- ] ; // ignore whitespace

\"(\\.|[^"\\])*\" { // String literal
    char* tmp=&yytext[1];
    while( *tmp != '"' )
    {
        ++tmp;
    }
    *tmp='\0';
    reference_init( yylval.ref );
    reference_create_str( yylval.ref, &yytext[1] );
    return(STRING);
    }

\'(\\.|[^'\\])*\' { // String literal
    char* tmp=&yytext[1];
    while( *tmp != '\'' )
    {
        ++tmp;
    }
    *tmp='\0';
    reference_init( yylval.ref );
    reference_create_str( yylval.ref, &yytext[1] );
    return(STRING);
    }

for        return(FOR);
while      return(WHILE);
in         return(IN);
if         return(IF);
else       return(ELSE);
elif       return(ELIF);
and        return(AND);
or         return(OR);
not        return(NOT);
exit       return(EXIT);
print      return(PRINT);
println    return(PRINTLN);

False {
    reference_init( yylval.ref );
    reference_create_bool( yylval.ref, 0);
    return(BOOL);
    }

True {
    reference_init( yylval.ref );
    reference_create_bool( yylval.ref, 1);
    return(BOOL);
    }

None {
    reference_init( yylval.ref );
    return(NONE);
    }

\[#([0-9a-zA-Z][0-9a-zA-Z])+\] { // Bytes literal
    intptr_t count=0;
    char* tmp=&yytext[2];
    char val;
    while( *tmp != ']' )
    {
        val = hexdigit(*tmp++) << 4;
        yytext[count++] = val + hexdigit(*tmp++);
    }
    reference_init( yylval.ref );
    reference_create_bytes( yylval.ref, yytext, count );
    return(BYTES);
    }

[a-zA-Z_][a-zA-Z0-9_]* { // Symbol name
    reference_init( yylval.ref );
    reference_create_str( yylval.ref, yytext );
    return(SYMBOL);
    }

0x[0-9a-zA-Z][0-9a-zA-Z]* {
    reference_init( yylval.ref );
    reference_create_int( yylval.ref, strtoll( yytext, NULL, 0) );
    return(NUMBER);
    }

[0-9]+ {
    reference_init( yylval.ref );
    reference_create_int( yylval.ref, strtoll( yytext, NULL, 0) );
    return(NUMBER);
    }

. { // Else pass characters as individual tokens
    return(yytext[0]);
    }

%%
