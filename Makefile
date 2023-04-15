
CFLAGS = -Wall -Wno-unused-function -Wno-main -Wvla
#-m32 -march=i686

all: calc smallscript hashtest

calc: calc.tab.c calc_lex.c calc.h smallscript.h hashtable.c Makefile
	gcc $(CFLAGS) -o calc calc.tab.c calc_lex.c hashtable.c

calc.tab.c calc.tab.h: calc.yy Makefile
	yacc -d -b calc -Wno-yacc -Wcounterexamples calc.yy

calc_lex.c: calc.lex Makefile
	lex --outfile=calc_lex.c calc.lex

smallscript: main.c smallc.h smallscript.c smallscript.h hashtable.c Makefile
	gcc $(CFLAGS) -o smallscript main.c hashtable.c
	#gcc $(CFLAGS) -o smallscript main.c smallscript.c

hashtest: hashtable.c hashtable.h smallc.h Makefile
	gcc $(CFLAGS) -DHASHTEST -o hashtest hashtable.c

clean-all:
	git clean -f
