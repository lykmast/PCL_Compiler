.PHONY: all

CXX=clang++
CC=clang
CXXFLAGS=-Wall -std=c++11 `llvm-config --cxxflags`
LDFLAGS:=`llvm-config --ldflags --system-libs --libs all`

SOURCES=pcl_lexer.cpp parser.cpp ast.cpp types.cpp \
	semantic.cpp library.cpp uid.cpp compile.cpp
OBJECTS=$(SOURCES:.cpp=.o)

all: pcl lib.o ## Build project (default choice).

debug: CXXFLAGS+= -g  ## Build project with debug options enabled.
debug: CXX=g++
debug: CC=gcc
debug: pcl

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

pcl_lexer.cpp: pcl_lexer.l
	flex -s -o pcl_lexer.cpp pcl_lexer.l

pcl_lexer.o: pcl_lexer.cpp pcl_lexer.hpp parser.hpp ast.hpp

parser.hpp parser.cpp: parser.y
	bison -d -o parser.cpp parser.y

ast.o: ast.hpp

parser.o: parser.hpp pcl_lexer.hpp ast.hpp

semantic.o: symbol.hpp ast.hpp

library.o: library.hpp ast.hpp

compile.o: ast.hpp cgen_table.hpp uid.hpp

lib.o: lib.c
	$(CC) -c -o $@ $<

pcl: $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(LDFLAGS)

clean:  ## Delete all automatically produced files, excluding final executable.
	$(RM) pcl_lexer.cpp parser.cpp parser.hpp *.o

distclean: clean ## Delete all automatically produced files, including final executable.
	$(RM) pcl

help:  ## Display this help.
	@awk 'BEGIN {FS = ":.*##"; printf "\nUsage:\n  make \033[36m<target>\033[0m\n\nTargets:\n"} /^[a-zA-Z_-]+:.*?##/ { printf "  \033[36m%-10s\033[0m %s\n", $$1, $$2 }' $(MAKEFILE_LIST)
