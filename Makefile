CC ?= cc
CFLAGS ?= -O2
LDFLAGS += -no-pie
# Change to macho64 on Mac, win64 on Windows
# (I don't know how to do conditionals like this in GNU Make, sorry)
NASM_FORMAT ?= elf64

ifeq ($(ENABLE_ASAN),1)
	LDFLAGS += -fsanitize=address
	CFLAGS += -fsanitize=address
endif

ifeq ($(ENABLE_UBSAN),1)
	LDFLAGS += -fsanitize=undefined
	CFLAGS += -fsanitize=undefined
endif

ifeq ($(DEBUG),1)
	CFLAGS += -g
	ASMFLAGS += -g
endif

objects = src/parser/ast.o src/parser/ast-free.o src/parser/ast-visit.o src/parser/main.o src/parser/pp.o

all: src/parser/main src/lexer/main

src/parser/main: $(objects) src/lexer/lexer.o

$(objects): %.o: %.c

src/lexer/lexer.o:
	nasm -f$(NASM_FORMAT) $(ASMFLAGS) src/lexer/lexer.asm

src/lexer/main: src/lexer/main.o src/lexer/lexer.o

clean:
	rm -f src/**/*.o src/lexer/main src/parser/main
