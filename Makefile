CC=g++
CFLAGS=-Wall
# compiler: compiler.lex.o compiler.tab.o symbol.o symbolc.o
# 	$(CC) $(CFLAGS) -o compiler compiler.lex.o compiler.tab.o \
# 	symbol.o symbolc.o
#
# compiler.lex.c: compiler.l compiler.tab.h
# 	flex -it compiler.l > compiler.lex.c
#
# compiler.tab.c compiler.tab.h: compiler.y
# 	bison -dv compiler.y
#
# clean:
# 	$(RM) *.o compiler.tab.c compiler.tab.h compiler.lex.c core
#
# distclean: clean$(RM) compiler

default: pcl
# pcl_lexer: pcl_lexer.cpp
# 	$(CC) -o pcl_lexer pcl_lexer.cpp

debug: CFLAGS+= -g
debug: pcl


pcl_lexer.cpp:pcl_lexer.l
	flex -s -o pcl_lexer.cpp pcl_lexer.l

pcl_lexer.o: pcl_lexer.cpp pcl_lexer.hpp parser.hpp ast.hpp symbol.hpp

parser.hpp parser.cpp: parser.y
	bison -dv -o parser.cpp parser.y

parser.o: parser.cpp pcl_lexer.hpp ast.hpp symbol.hpp

pcl: pcl_lexer.o parser.o
	$(CC) $(CFLAGS) -o pcl pcl_lexer.o parser.o

clean:
	$(RM) pcl_lexer.cpp parser.cpp parser.hpp parser.output *.o

distclean: clean
	$(RM) pcl
