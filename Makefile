CC=gcc
CFLAGS=-Wall
compiler: compiler.lex.o compiler.tab.o symbol.o symbolc.o
	$(CC) $(CFLAGS) -o compiler compiler.lex.o compiler.tab.o \
	symbol.o symbolc.o

compiler.lex.c: compiler.l compiler.tab.h
	flex -it compiler.l > compiler.lex.c

compiler.tab.c compiler.tab.h: compiler.y
	bison -dv compiler.y

clean:
	$(RM) *.o compiler.tab.c compiler.tab.h compiler.lex.c core

distclean: clean$(RM) compiler
