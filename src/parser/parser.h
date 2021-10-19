#pragma once

#include "../lexer/lexer.h"
#include "ast.h"

typedef enum {
    PARSE_BAD = 0,
    PARSE_OK = 1,
    PARSE_OK_NOTHING,
} pres_t;

pres_t parse_program(const char** stream, program_t* program);
pres_t parse_decl(const char** stream, decl_t* decl);