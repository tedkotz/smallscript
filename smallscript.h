#ifndef SMALLSCRIPT_H
#define SMALLSCRIPT_H
/*
 * typedef struct sesc_attr sesc_attr;
 * typedef struct sesc_context sesc_context;
 */
typedef intptr_t sesc_attr;

typedef intptr_t sesc_context;

extern sesc_attr* sesc_attr_create(void);
extern sesc_context* sesc_context_create(sesc_attr* attr);
extern intptr_t sesc_eval_string(sesc_context* ctx, const char * str);
extern intptr_t sesc_get_int_by_idx(sesc_context* ctx, intptr_t id);
extern intptr_t sesc_get_int_by_name(sesc_context* ctx, const char* name);
extern void sesc_context_destroy(sesc_context* ctx);
extern void sesc_attr_destroy(sesc_attr* ctx);

/*
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
// <biop> ::= [+-/%*^&] | '//' | '<<' | '>>'
// <uop> ::= [!~]
*/

#endif
