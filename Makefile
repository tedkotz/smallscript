

calc: y.tab.c lex.yy.c calc.h smallscript.h hashtable.c
	gcc -Wall -Wno-unused-function -o calc y.tab.c lex.yy.c hashtable.c

y.tab.c y.tab.h: calc.yy
	yacc -d -Wno-yacc calc.yy

lex.yy.c: calc.lex
	lex calc.lex

smallscript: main.c smallc.h smallscript.c smallscript.h hashtable.c
	gcc -Wall -Wno-main -m32 -o smallscript main.c hashtable.c
	#gcc -Wall -Wno-main -o smallscript main.c smallscript.c
	#gcc -Wall -Wno-main -m32 -march=i686 -o smallscript main.c smallscript.c

hashtest: hashtable.c hashtable.h smallc.h
	gcc -Wall -Wno-main -DHASHTEST -m32 -o hashtest hashtable.c

clean-all:
	git clean -f
