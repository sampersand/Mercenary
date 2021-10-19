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
    free(decl->args);
    visitor->visit_block(visitor, decl->block);
}

void free_program_post(ast_visitor_t* visitor, program_t* program) {
    if (program->len > 0) {
        free(program->decls);
    }
    free(program);
}

void free_decl_post(ast_visitor_t* visitor, decl_t* decl) {
    free(decl);
}

void free_block_post(ast_visitor_t* visitor, block_t* block) {
    free(block);
}

void free_stmt_post(ast_visitor_t* visitor, stmt_t* stmt) {
    free(stmt);
}

void free_expr_post(ast_visitor_t* visitor, expr_t* expr) {
    free(expr);
}

void free_call_expr(ast_visitor_t* visitor, call_expr_t* expr) {
    visitor->visit_expr(visitor, expr->func);
    for (int i = 0; i < expr->arity; i++) {
        visitor->visit_expr(visitor, expr->args[i]);
    }
    if (expr->arity > 0) {
        free(expr->args);
    }
}

void free_array_expr(ast_visitor_t* visitor, array_expr_t* expr) {
    for (int i = 0; i < expr->len; i++) {
        visitor->visit_expr(visitor, expr->exprs[i]);
    }
    if (expr->len > 0) {
        free(expr->exprs);
    }
    free(expr->exprs);
}

void free_if_stmt(ast_visitor_t* visitor, if_stmt_t* stmt) {
    visitor->visit_expr(visitor, stmt->main_cond);
    visitor->visit_block(visitor, stmt->main_block);
    for (int i = 0; i < stmt->elif_len; i++) {
        visitor->visit_expr(visitor, stmt->elif_conds[i]);
        visitor->visit_block(visitor, stmt->elif_blocks[i]);
    }
    if (stmt->elif_len > 0) {
        free(stmt->elif_conds);
        free(stmt->elif_blocks);
    }
    if (stmt->else_block != NULL) {
        visitor->visit_block(visitor, stmt->else_block);
    }
}

void free_block_visitor(ast_visitor_t* visitor, block_t* block) {
    visitor->visit_block_pre(visitor, block);
    for (int i = 0; i < block->len; i++) {
        visitor->visit_stmt(visitor, block->stmts[i]);
    }
    if (block->len > 0) {
        free(block->stmts);
    }
    visitor->visit_block_post(visitor, block);
}

ast_visitor_t* free_visitor() {
    ast_visitor_t* visitor = malloc(sizeof(ast_visitor_t));
    memcpy(visitor, &DEFAULT_VISITOR, sizeof(ast_visitor_t));
    visitor->visit_fn_decl = free_fn_decl;
    visitor->visit_program_post = free_program_post;
    visitor->visit_decl_post = free_decl_post;
    visitor->visit_block_post = free_block_post;
    visitor->visit_stmt_post = free_stmt_post;
    visitor->visit_expr_post = free_expr_post;
    visitor->visit_call_expr = free_call_expr;
    visitor->visit_array_expr = free_array_expr;
    visitor->visit_if_stmt = free_if_stmt;
    visitor->visit_block = free_block_visitor;

    return visitor;
}

// void free_expr(expr_t* expr) {
//     ast_visitor_t* visitor = free_visitor();
//     visitor->visit_expr(visitor, expr);
//     free(visitor);
// }

// void free_stmt(stmt_t* stmt) {
//     ast_visitor_t* visitor = free_visitor();
//     visitor->visit_stmt(visitor, stmt);
//     free(visitor);
// }

// void free_block(block_t* block) {
//     ast_visitor_t* visitor = free_visitor();
//     visitor->visit_block(visitor, block);
//     free(visitor);
// }

// void free_decl(decl_t* decl) {
//     ast_visitor_t* visitor = free_visitor();
//     visitor->visit_decl(visitor, decl);
//     free(visitor);
// }

void free_program(program_t* program) {
    ast_visitor_t* visitor = free_visitor();
    visitor->visit_program(visitor, program);
    free(visitor);
}