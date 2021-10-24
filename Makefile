CC ?= cc
CFLAGS ?= -O2
# CFLAGS += -Isrc/parser -Isrc/lexer
LDFLAGS += -no-pie
# Change to macho64 on Mac, win64 on Windows
# (I don't know how to do conditionals like this in GNU Make, sorry)
NASM_FORMAT ?= elf64

CXX ?= g++
LDLIBS ?= -lstdc++ -lm
CXXFLAGS ?= -std=c++17 -L/usr/include/boost -lboost_filesystem

ifeq ($(ASAN),1)
	LDFLAGS += -fsanitize=address
	CFLAGS += -fsanitize=address
endif

ifeq ($(UBSAN),1)
	LDFLAGS += -fsanitize=undefined
	CFLAGS += -fsanitize=undefined
endif

ifeq ($(DEBUG),1)
	CFLAGS += -g
	ASMFLAGS += -g
endif

parser_objs = src/parser/ast-free.o src/parser/ast-visit.o src/parser/ast.o src/parser/pp.o src/parser/parser.o

all: src/parser/main src/lexer/main src/codegen/main

src/parser/main: $(parser_objs) src/lexer/lexer.o

$(parser_objs): %.o: %.c

src/lexer/lexer.o: src/lexer/lexer.asm
	nasm -f$(NASM_FORMAT) $(ASMFLAGS) src/lexer/lexer.asm

src/lexer/main: src/lexer/main.o src/lexer/lexer.o

codegen_objs = src/codegen/ast.o

$(codegen_objs): %.o: %.cpp

src/codegen/main: src/codegen/main.o $(codegen_objs) src/lexer/lexer.o $(parser_objs)

clean:
	rm -f src/**/*.o src/lexer/main src/parser/main
