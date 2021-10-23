#include <stdlib.h>
#include <string.h>

#include "ast-visit.h"
#include "ast.h"
void free_fn_decl(ast_visitor_t* visitor, fn_decl_t* decl) {
  arr_free(decl->args);
  visitor->visit_block(visitor, &decl->block);
}

void free_program_post(ast_visitor_t* visitor, program_t* program) {
  arr_free(*program);
}

void free_block_post(ast_visitor_t* visitor, block_t* block) {
  arr_free(*block);
}

void free_expr_post(ast_visitor_t* visitor, expr_t* expr) {
  if (expr->boxed) {
    free(expr);
  }
}

void free_call_expr(ast_visitor_t* visitor, call_expr_t* expr) {
  visitor->visit_expr(visitor, expr->func);
  for (int i = 0; i < arr_get_size(expr->args); i++) {
    visitor->visit_expr(visitor, &arr_at(expr->args, i));
  }
  arr_free(expr->args);
}

void free_array_expr(ast_visitor_t* visitor, expr_array_t* exprs) {
  for (int i = 0; i < arr_get_size(*exprs); i++) {
    visitor->visit_expr(visitor, &arr_at(*exprs, i));
  }
  arr_free(*exprs);
}

void free_if_stmt(ast_visitor_t* visitor, if_stmt_t* stmt) {
  visitor->visit_expr(visitor, stmt->main_cond);
  visitor->visit_block(visitor, &stmt->main_block);
  for (int i = 0; i < arr_get_size(stmt->elif_conds); i++) {
    visitor->visit_expr(visitor, &arr_at(stmt->elif_conds, i));
    visitor->visit_block(visitor, &arr_at(stmt->elif_blocks, i));
  }
  arr_free(stmt->elif_conds);
  arr_free(stmt->elif_blocks);
  if (stmt->else_block != NULL) {
    visitor->visit_block(visitor, &stmt->else_block);
  }
}

void free_assign_var(ast_visitor_t* visitor, assign_var_t* stmt) {
  for (int i = 0; i < arr_get_size(stmt->indices); i++) {
    visitor->visit_expr(visitor, &arr_at(stmt->indices, i));
  }
  arr_free(stmt->indices);
  visitor->visit_expr(visitor, stmt->value);
}

ast_visitor_t free_visitor() {
  ast_visitor_t visitor = DEFAULT_VISITOR;
  visitor.visit_fn_decl = free_fn_decl;
  visitor.visit_program_post = free_program_post;
  visitor.visit_block_post = free_block_post;
  visitor.visit_expr_post = free_expr_post;
  visitor.visit_call_expr = free_call_expr;
  visitor.visit_array_expr = free_array_expr;
  visitor.visit_if_stmt = free_if_stmt;
  visitor.visit_assign_var = free_assign_var;

  return visitor;
}

void free_program(program_t program) {
  ast_visitor_t visitor = free_visitor();
  visitor.visit_program(&visitor, &program);
}

void free_block(block_t block) {
  ast_visitor_t visitor = free_visitor();
  visitor.visit_block(&visitor, &block);
}

void free_expr(expr_t expr) {
  ast_visitor_t visitor = free_visitor();
  visitor.visit_expr(&visitor, &expr);
}