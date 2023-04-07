#ifndef SMALLSCRIPT_H
#define SMALLSCRIPT_H

typedef struct sscript_attr sscript_attr;

typedef struct sscript_context sscript_context;

extern sscript_attr* sscript_attr_create(void);
extern sscript_context* sscript_context_create(sscript_attr* attr);
extern int sscript_eval_string(sscript_context* ctx, const char * str);
extern int sscript_get_int(sscript_context* ctx, int id);
extern const char* sscript_get_string(sscript_context* ctx, int id);
extern void sscript_context_destroy(sscript_context* ctx);
extern void sscript_attr_destroy(sscript_attr* ctx);


// [ "ant", "is", "uncle"][1:3][0]
// is

// <body> ::= <expr>|<expr> ';' <body>
// <expr> ::= <lvalue> '=' <expr> |
//            <value>|
//            <uop> <value> |
//            <expr> <biop> <value>|
//            <list> <list> |
//            'for' <var> 'in' <list> '{' <body> '}' |
//            'while' <expr> '{' <body> '}' |
//            'if' <expr> '{' <body> '}' |
//            'if' <expr> '{' <body> '}' else '{' <body> '}' |
//            'print(' <expr> ')'
// <value> ::= <lvalue> | '('<expr> ')' | <funccall>
// <lvalue> ::= <var> | <lvalue> <list> | <var>'.'<lvalue>
// <var> ::= r"[a-zA-Z_][a-zA-Z0-9_]"
// <list> ::= <lvalue> | '[' <listbody> ']' | '[]' | <funccall>
// <object> ::= <lvalue> | '{' <objectbody> '}' | '{}'| <funccall>
// <function> ::= <lvalue> | <funccall> | 'func' <varlist> { <body> }
// <funccall> ::= <function> <list>
// <biop> ::= [+-/*%^&] | '//' | '<<' | '>>'
// <uop> ::= [!~]

#endif
