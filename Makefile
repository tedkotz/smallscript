

smallscript: main.c smallc.h smallscript.c smallscript.h
	gcc -Wall -Wno-main -o smallscript main.c smallscript.c
	#gcc -Wall -Wno-main -m32 -march=i686 -o smallscript main.c smallscript.c

