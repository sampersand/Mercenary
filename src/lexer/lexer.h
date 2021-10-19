#pragma once

#include <stdint.h>

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