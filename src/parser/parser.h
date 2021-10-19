#pragma once

#include "../lexer/lexer.h"
#include "ast.h"

program_t* parse_program(const char* stream);