CC ?= cc
CFLAGS ?= -O2
# CFLAGS += -Isrc/parser -Isrc/lexer
ARCH := $(shell uname -m)
ifeq ($(filter-out arm64 aarch64,$(ARCH)),)
    # arm64
    lexer_obj := src/lexer/lexer_arm64.o
else
    LDFLAGS += -no-pie
    # assume x86_64
    lexer_obj := src/lexer/lexer.o
    ifeq ($(OS),Windows_NT)
        # Use win64 for Windows only
        NASM_FORMAT = win64
    else
        UNAME_S := $(shell uname -s)
        ifeq ($(UNAME_S),Darwin)
            # Use macho64 for Darwin only
            NASM_FORMAT = macho64
        else
            # Use elf64 for everything else
            NASM_FORMAT = elf64
        endif
    endif
endif

CXX ?= g++
LDLIBS ?= -lstdc++ -lm
CXXFLAGS ?= -std=c++2a

ifeq ($(ASAN),1)
	LDFLAGS += -fsanitize=address
	CFLAGS += -fsanitize=address
	CXXFLAGS += -fsanitize=address
endif

ifeq ($(UBSAN),1)
	LDFLAGS += -fsanitize=undefined
	CFLAGS += -fsanitize=undefined
	CXXFLAGS += -fsanitize=undefined
endif

ifeq ($(DEBUG),1)
	CFLAGS += -g
	ASMFLAGS += -g
	CXXFLAGS += -g
endif

parser_objs = src/parser/ast-free.o src/parser/ast-visit.o src/parser/ast.o src/parser/pp.o src/parser/parser.o

all: src/parser/main src/lexer/main src/codegen/main

src/parser/main: $(parser_objs) $(lexer_obj)

$(parser_objs): %.o: %.c

src/lexer/lexer.o: src/lexer/lexer.asm
	nasm -f$(NASM_FORMAT) $(ASMFLAGS) src/lexer/lexer.asm
src/lexer/lexer_arm64.o: src/lexer/lexer_arm64.s
	$(AS) $(ASMFLAGS) $< -o $@

src/lexer/main: src/lexer/main.o $(lexer_obj)

codegen_objs = src/codegen/ast.o src/codegen/middle_end.o src/codegen/instructions.o

src/codegen/main: src/codegen/main.o $(codegen_objs) $(lexer_obj) $(parser_objs)

clean:
	rm -f src/**/*.o src/lexer/main src/parser/main src/codegen/main
