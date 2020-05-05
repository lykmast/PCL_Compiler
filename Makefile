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
	bison -d -o parser.cpp parser.y

parser.o: parser.cpp pcl_lexer.hpp ast.hpp ast.cpp symbol.hpp

semantic.o:semantic.cpp symbol.hpp
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.cpp ast.hpp
	$(CC) $(CFLAGS) -c -o $@ $<

pcl: pcl_lexer.o parser.o ast.o semantic.o runtime.o types.o library.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	$(RM) pcl_lexer.cpp parser.cpp parser.hpp parser.output *.o

distclean: clean
	$(RM) pcl
