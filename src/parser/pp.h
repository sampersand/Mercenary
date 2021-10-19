#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ast.h"
#include "ast-visit.h"

void pp_program(program_t program);
// void pp_decl(decl_t* decl);
// void pp_stmt(stmt_t* stmt);
// void pp_expr(expr_t* expr);