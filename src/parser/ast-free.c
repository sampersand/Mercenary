#include "ast.h"
#include <stdlib.h>
#include <string.h>
#include "ast-visit.h"

// #define PLURAL_WRAPPER(singular, plural) \
//     void free_##plural(uint32_t len, singular##_t** plural) { \
//         for (int i = 0; i < len; i++) { \
//             free_##singular(plural[i]); \
//         } \
//         free(plural); \
//     }

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
    free(expr);
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
        visitor->visit_block(visitor, stmt->else_block);
        free(stmt->else_block);
    }
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

    return visitor;
}

void free_program(program_t program) {
    ast_visitor_t visitor = free_visitor();
    visitor.visit_program(&visitor, &program);
}