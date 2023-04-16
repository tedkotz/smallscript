
CFLAGS = --ansi -Wall -Wextra -Wno-ignored-qualifiers -Wno-unused-function -Wno-main -Wvla -m32 -march=i686
#-mx32 -march=x86-64
#-m32 -march=i686
# -Wtraditional
# --std=c99 -D_GNU_SOURCE

all: calc smallscript hashtest synparse

synparse: syntax.tab.c sesc_lex.c sesc_lex.h smallscript.h hashtable.c Makefile
	gcc $(CFLAGS) -o synparse syntax.tab.c sesc_lex.c hashtable.c

syntax.tab.c syntax.tab.h: syntax.yy Makefile
	yacc -d -b syntax -Wno-yacc -Wcounterexamples syntax.yy

calc: calc.tab.c sesc_lex.c sesc_lex.h smallscript.h hashtable.c Makefile
	gcc $(CFLAGS) -o calc -DCALC calc.tab.c sesc_lex.c hashtable.c

calc.tab.c calc.tab.h: calc.yy Makefile
	yacc -d -b calc -Wno-yacc -Wcounterexamples calc.yy

sesc_lex.c sesc_lex.h: syntax.lex Makefile
	lex --hex -l --outfile=sesc_lex.c --header-file=sesc_lex.h syntax.lex

smallscript: main.c smallc.h smallscript.c smallscript.h hashtable.c Makefile
	gcc $(CFLAGS) -o smallscript main.c hashtable.c
	#gcc $(CFLAGS) -o smallscript main.c smallscript.c

hashtest: hashtable.c hashtable.h smallc.h Makefile
	gcc $(CFLAGS) -DHASHTEST -o hashtest hashtable.c

clean-all:
	git clean -f
