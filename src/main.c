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

typedef struct Token {
    const char* start;
    const char* end;
    uint64_t type;
} Token;

enum {
    TOKEN_ERROR = 256,
    TOKEN_IDENTIFIER = 257,
    TOKEN_NUMBER = 258,
    TOKEN_STRING = 259,
    
    TOKEN_DOUBLE_EQUALS = 260,
    TOKEN_NOT_EQUALS = 261,
    TOKEN_GREATER_EQUALS = 262,
    TOKEN_LESSER_EQUALS = 263,
    
    TOKEN_GLOBAL = 300,
    TOKEN_FUNCTION = 301,
    TOKEN_IF = 302,
    TOKEN_ELSE = 303,
    TOKEN_WHILE = 304,
    TOKEN_RETURN = 305,
    TOKEN_DO = 306,
    TOKEN_LET = 307,
    TOKEN_TRUE = 308,
    TOKEN_FALSE = 309,
    TOKEN_IMPORT = 310,
    TOKEN_NULL = 311
};

#ifdef _WIN32
extern Token read_token(const char* lexer);
#else
extern Token read_token_unix(const char* lexer);
#define read_token read_token_unix
#endif // _WIN32

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
    
	fseek(file, 0, SEEK_SET);
	fread(file_data, 1, length, file);
    file_data[length] = 0; // just in case it's a text file
	fclose(file);
    
    return file_data;
}

int main(int argc, char** argv) {
    if (argc <= 1) {
        printf("No input file!\n");
        return -1;
    }
    
    char* file = read_entire_file(argv[1]);
    if (file == NULL) {
        printf("Could not read file!\n");
        return -1;
    }
    
    const char* stream = file;
    
    while (true) {
        Token t = read_token(stream);
        stream = t.end;
        
        printf("TYPE=%03d\t'%.*s'\n", (int)t.type, (int)(t.end - t.start), t.start);
        if (t.type == 0) break;
    }
    
    free(file);
    return 0;
}
