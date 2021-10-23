#include "ast.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_string(string_t str) { printf("%2$.*1$s", str.len, str.s); }

string_t mk_string(const char* s, uint32_t len) {
  return (string_t){.len = len, .s = s};
}

string_t mk_string_cstr(const char* s) {
  size_t len = strlen(s);
  assert(len <= (size_t)(uint32_t)-1);
  return mk_string(s, strlen(s));
}

string_t mk_string_2ptrs(const char* start, const char* end) {
  size_t len = end - start;
  assert(len <= (size_t)(uint32_t)-1);
  return mk_string(start, end - start);
}

expr_t* box_expr(expr_t expr) {
  expr_t* new_expr = (expr_t*)malloc(sizeof(expr_t));
  *new_expr = expr;
  new_expr->boxed = true;
  return new_expr;
}

expr_t mk_binop(expr_t lhs, expr_t rhs, binop_t op) {
  return (expr_t){
      .kind = EXPR_BINARY,
      .value.binary =
          (binop_expr_t){.lhs = box_expr(lhs), .rhs = box_expr(rhs), .op = op},
      .boxed = false};
}

expr_t mk_unop(expr_t subexpr, unop_t op) {
  return (expr_t){
      .kind = EXPR_UNARY,
      .value.unary = (unop_expr_t){.subexpr = box_expr(subexpr), .op = op},
      .boxed = false};
}

expr_t mk_call(expr_t func, expr_array_t args) {
  return (expr_t){.kind = EXPR_CALL,
                  .value.call =
                      (call_expr_t){
                          .func = box_expr(func),
                          .args = args,
                      },
                  .boxed = false};
}

expr_t mk_index(expr_t array, expr_t index) {
  return (expr_t){.kind = EXPR_INDEX,
                  .value.index = (index_expr_t){.array = box_expr(array),
                                                .index = box_expr(index)},
                  .boxed = false};
}

expr_t mk_array(expr_array_t exprs) {
  return (expr_t){.kind = EXPR_ARRAY, .value.array = exprs, .boxed = false};
}

expr_t mk_bool(bool val) {
  return (expr_t){.kind = EXPR_BOOL, .value.bool_expr = val, .boxed = false};
}

expr_t mk_number(uint64_t number) {
  return (expr_t){.kind = EXPR_NUMBER, .value.number = number, .boxed = false};
}

expr_t mk_null() {
  return (expr_t){.kind = EXPR_NULL, .value = 0, .boxed = false};
}

expr_t mk_string_expr(string_t string) {
  return (expr_t){.kind = EXPR_STRING, .value.string = string, .boxed = false};
}

expr_t mk_ident(string_t ident) {
  return (expr_t){.kind = EXPR_IDENT, .value.string = ident, .boxed = false};
}

stmt_t mk_if(expr_t main_cond, block_t main_block, expr_array_t elif_conds,
             block_array_t elif_blocks, block_t else_block) {
  assert((arr_get_size(elif_conds) == arr_get_size(elif_blocks)));
  return (stmt_t){.kind = STMT_IF,
                  .value.if_stmt = (if_stmt_t){.main_cond = box_expr(main_cond),
                                               .main_block = main_block,
                                               .elif_conds = elif_conds,
                                               .elif_blocks = elif_blocks,
                                               .else_block = else_block}};
}

stmt_t mk_while(expr_t cond, block_t block) {
  return (stmt_t){.kind = STMT_WHILE,
                  .value.while_stmt = (while_stmt_t){
                      .cond = box_expr(cond),
                      .block = block,
                  }};
}

stmt_t mk_return(expr_t expr) {
  return (stmt_t){.kind = STMT_RETURN, .value.expr = box_expr(expr)};
}

stmt_t mk_declare_var(string_t ident, expr_t value) {
  return (stmt_t){.kind = STMT_DECLARE_VAR,
                  .value.declare_var = (declare_var_t){
                      .ident = ident, .value = box_expr(value)}};
}

stmt_t mk_assign_var(string_t ident, expr_array_t indexes, expr_t value) {
  return (stmt_t){
      .kind = STMT_ASSIGN_VAR,
      .value.assign_var = (assign_var_t){
          .ident = ident, .indices = indexes, .value = box_expr(value)}};
}

stmt_t mk_do(expr_t expr) {
  return (stmt_t){.kind = STMT_DO, .value = box_expr(expr)};
}

decl_t mk_fn_decl(string_t name, string_array_t args, block_t block) {
  return (decl_t){
      .kind = DECL_FUNCTION,
      .value.fn = (fn_decl_t){.name = name, .args = args, .block = block}};
}

decl_t mk_global(string_t ident) {
  return (decl_t){.kind = DECL_GLOBAL, .value.string = ident};
}

decl_t mk_import(string_t string) {
  return (decl_t){.kind = DECL_IMPORT, .value.string = string};
}