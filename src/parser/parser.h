#pragma once

#include "../lexer/lexer.h"
#include "ast.h"

typedef enum {
  PARSE_BAD = 0,
  PARSE_OK = 1,
  PARSE_OK_NOTHING,
} pres_t;

arr_forward_decl(uint32_array_t) arr_decl(uint32_array_t, uint32_t)

    typedef struct {
  const char* stream_start;
  uint32_t overall_len;
  uint32_array_t line_offsets;
} eh_data_t;

pres_t parse_program(const char** stream, program_t* program, eh_data_t eh);
pres_t parse_decl(const char** stream, decl_t* decl, eh_data_t eh);
pres_t parse_block(const char** stream, block_t* block, eh_data_t eh);
pres_t parse_stmt(const char** stream, stmt_t* stmt, eh_data_t eh);
pres_t parse_expr(const char** stream, expr_t* expr, eh_data_t eh);

uint32_t line_num(eh_data_t eh, uint32_t offset);
uint32_t col_num(eh_data_t eh, uint32_t offset);
uint32_array_t mk_offsets_list(const char* stream, uint32_t len);