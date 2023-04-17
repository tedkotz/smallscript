smallscript
===========

Small embeddable scripting language (SESC)

Design Goals
------------
- JSON compatible object notation
- Small
    - Small executable footprint
    - Small memory footprint
    - Small-C compatible
    - Small language set
- Consistency
    - every thing should behave symmetrically
    - ranges should act like lists
- Inspirations
    - Javascript
        - Duktape
    - Lua
    - Python

Thoughts & Todo
---------------
- [ ] Needs Callable functionality
    - ABI:
        - Args are passed and returns as integer index in current scope
        - Args are passed as list `argv` in current scope
    - Parent Scope reference:
        - integer index 0 in current scope (except root scope)
        - called `parent`
    - Root Scope
        - called `global`
        - called `root`
        - called `parent`
    - Callable code bodies
        - Looping
        - Conditionals
        - function definitions
        - save as
            - raw text
                - define ctx and run with eval
            - minified text
                - runs as raw, but more efficient
            - compiled byte code
                - requires byte code parser
                - moves toward full compilation
            - executable parse tree
                - just walk the tree to run the code
                - similar to test approaches, but stored first half of eval already done.


- [ ] Needs Small-C compatible interpreter

- [ ] Memory management
    - Rely on reference counting
    - Needs way to detect circular references
        - Scan contents of htable for itself?
        - Prevent an htable from containing itself as key or value?

- [ ] Needs iterators
    - Lists as:
         - resizable array type
         - Just a htable
    - how to iterate through keys

- [ ] a better way to uniquely save strings

- [ ] save memory

- should hashs of htables hash contents?

- operators
    - just call into hash lookup function
    - cfunc for each operator
    - should unary be left associative? does it matter?
    - Implicit Type conversions
    - explicit type conversions
    - function overloading through recursive context stack
    - operator precedence
        - want less levels than C, python, javascript or lua
        - Pascal model makes some sense
    - others
        - `iff` logical equality
        - `implies` logical
        - `xor` logical sum
