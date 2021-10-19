#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include <sys/stat.h>

#ifdef _WIN32
#define fileno _fileno
#define fstat _fstat
#define stat _stat
#endif

#include "ast.h"
#include "pp.h"
#include "parser.h"

#include "../lexer/lexer.h"

char* read_entire_file(const char* filepath) {
    FILE* file = fopen(filepath, "rb");
    int descriptor = fileno(file);
    
    struct stat file_stats;
    if (fstat(descriptor, &file_stats) == -1) return NULL;
    
    // needs 4 extra bytes of slack space 
    // for the lexer not to crash identifying 
    // keywords at the end of the string
    int length = file_stats.st_size;
	char* file_data = malloc(length + 4);
    memset(file_data, 0, length + 4);
    
	fseek(file, 0, SEEK_SET);
	fread(file_data, 1, length, file);
    file_data[length] = 0; // just in case it's a text file
	fclose(file);
    
    return file_data;
}

int main(int argc, char** argv) {
    if (argc <= 1) {
        fputs("No input file!\n", stderr);
        return -1;
    }
    
    char* file = read_entire_file(argv[1]);
    if (file == NULL) {
        fputs("Could not read file!\n", stderr);
        return -1;
    }
    
    const char* stream = file;
    const char* error = NULL;

    program_t program;
    
    pres_t res = parse_program(&stream, &program);
    
    if (!res) {
        return -1;
    }

    pp_program(program);
    free_program(program);

    free(file);
    // while (true) {
    //     Token t = read_token(stream);
    //     stream = t.end;
        
    //     printf("TYPE=%03d\t'%.*s'\n", (int)t.type, (int)(t.end - t.start), t.start);
    //     if (t.type == 0) break;
    // }
    
    return 0;
}