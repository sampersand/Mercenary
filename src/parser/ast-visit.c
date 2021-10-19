#include "ast-visit.h"
#include <stdlib.h>

void walk_binop_expr(ast_visitor_t* visitor, binop_expr_t* expr) {
    visitor->visit_expr(visitor, expr->lhs);
    visitor->visit_expr(visitor, expr->rhs);
}
void walk_unop_expr(ast_visitor_t* visitor, unop_expr_t* expr) {
    visitor->visit_expr(visitor, expr->subexpr);
}
void walk_call_expr(ast_visitor_t* visitor, call_expr_t* expr) {
    visitor->visit_expr(visitor, expr->func);
    for (int i = 0; i < expr->arity; i++) {
        visitor->visit_expr(visitor, expr->args[i]);
    }
}
void walk_index_expr(ast_visitor_t* visitor, index_expr_t* expr) {
    visitor->visit_expr(visitor, expr->array);
    visitor->visit_expr(visitor, expr->index);
}
void walk_bool_expr(ast_visitor_t* visitor, bool* expr) {}
void walk_number_expr(ast_visitor_t* visitor, uint64_t* expr) {}
void walk_string_expr(ast_visitor_t* visitor, string_t* expr) {}
void walk_ident_expr(ast_visitor_t* visitor, string_t* expr) {}
void walk_array_expr(ast_visitor_t* visitor, array_expr_t* expr) {
    for (int i = 0; i < expr->len; i++) {
        visitor->visit_expr(visitor, expr->exprs[i]);
    }
}
void walk_null_expr(ast_visitor_t* visitor) {}
void walk_expr(ast_visitor_t* visitor, expr_t* expr) {
    visitor->visit_expr_pre(visitor, expr);
    switch (expr->kind) {
        case EXPR_BINARY: {
            visitor->visit_binop_expr(visitor, &expr->value.binary);
            break;
        }
        case EXPR_UNARY: {
            visitor->visit_unop_expr(visitor, &expr->value.unary);
            break;
        }
        case EXPR_CALL: {
            visitor->visit_call_expr(visitor, &expr->value.call);
            break;
        }
        case EXPR_INDEX: {
            visitor->visit_index_expr(visitor, &expr->value.index);
            break;
        }
        case EXPR_BOOL: {
            visitor->visit_bool_expr(visitor, &expr->value.bool_expr);
            break;
        }
        case EXPR_NUMBER: {
            visitor->visit_number_expr(visitor, &expr->value.number);
            break;
        }
        case EXPR_STRING: {
            visitor->visit_string_expr(visitor, &expr->value.string);
            break;
        }
        case EXPR_IDENT: {
            visitor->visit_ident_expr(visitor, &expr->value.string);
            break;
        }
        case EXPR_ARRAY: {
            visitor->visit_array_expr(visitor, &expr->value.array);
            break;
        }
        case EXPR_NULL: {
            visitor->visit_null_expr(visitor);
            break;
        }
    }
    visitor->visit_expr_post(visitor, expr);
}
void walk_expr_pre(ast_visitor_t* visitor, expr_t* expr) {}
void walk_expr_post(ast_visitor_t* visitor, expr_t* expr) {}

void walk_if_stmt(ast_visitor_t* visitor, if_stmt_t* stmt) {
    visitor->visit_expr(visitor, stmt->main_cond);
    visitor->visit_block(visitor, stmt->main_block);
    for (int i = 0; i < stmt->elif_len; i++) {
        visitor->visit_expr(visitor, stmt->elif_conds[i]);
        visitor->visit_block(visitor, stmt->elif_blocks[i]);
    }
    if (stmt->else_block != NULL) {
        visitor->visit_block(visitor, stmt->else_block);
    }
}
void walk_while_stmt(ast_visitor_t* visitor, while_stmt_t* stmt) {
    visitor->visit_expr(visitor, stmt->cond);
    visitor->visit_block(visitor, stmt->block);
}
void walk_return_stmt(ast_visitor_t* visitor, expr_t* stmt) {
    visitor->visit_expr(visitor, stmt);
}
void walk_do_stmt(ast_visitor_t* visitor, expr_t* stmt) {
    visitor->visit_expr(visitor, stmt);
}
void walk_assign_normal(ast_visitor_t* visitor, assign_normal_t* stmt) {
    visitor->visit_expr(visitor, stmt->value);
}
void walk_assign_array(ast_visitor_t* visitor, assign_array_t* stmt) {
    visitor->visit_expr(visitor, stmt->array);
    visitor->visit_expr(visitor, stmt->index);
    visitor->visit_expr(visitor, stmt->value);
}
void walk_global_assign_normal(ast_visitor_t* visitor, assign_normal_t* stmt) {
    visitor->visit_expr(visitor, stmt->value);
}
void walk_global_assign_array(ast_visitor_t* visitor, assign_array_t* stmt) {
    visitor->visit_expr(visitor, stmt->array);
    visitor->visit_expr(visitor, stmt->index);
    visitor->visit_expr(visitor, stmt->value);
}
void walk_stmt(ast_visitor_t* visitor, stmt_t* stmt) {
    visitor->visit_stmt_pre(visitor, stmt);
    switch (stmt->kind) {
        case STMT_IF: {
            visitor->visit_if_stmt(visitor, &stmt->value.if_stmt);
            break;
        }
        case STMT_WHILE: {
            visitor->visit_while_stmt(visitor, &stmt->value.while_stmt);
            break;
        }
        case STMT_RETURN: {
            visitor->visit_return_stmt(visitor, stmt->value.expr);
            break;
        }
        case STMT_ASSIGN_NORMAL: {
            visitor->visit_assign_normal(visitor, &stmt->value.assign_normal);
            break;
        }
        case STMT_ASSIGN_ARRAY: {
            visitor->visit_assign_array(visitor, &stmt->value.assign_array);
            break;
        }
        case STMT_GLOBAL_ASSIGN_NORMAL: {
            visitor->visit_global_assign_normal(visitor, &stmt->value.assign_normal);
            break;
        }
        case STMT_GLOBAL_ASSIGN_ARRAY: {
            visitor->visit_global_assign_array(visitor, &stmt->value.assign_array);
            break;
        }
        case STMT_DO: {
            visitor->visit_do_stmt(visitor, stmt->value.expr);
            break;
        }
    }
    visitor->visit_stmt_post(visitor, stmt);
}
void walk_stmt_pre(ast_visitor_t* visitor, stmt_t* stmt) {}
void walk_stmt_post(ast_visitor_t* visitor, stmt_t* stmt) {}

void walk_block(ast_visitor_t* visitor, block_t* block) {
    visitor->visit_block_pre(visitor, block);
    for (int i = 0; i < block->len; i++) {
        visitor->visit_stmt(visitor, block->stmts[i]);
    }
    visitor->visit_block_post(visitor, block);
}
void walk_block_pre(ast_visitor_t* visitor, block_t* block) {}
void walk_block_post(ast_visitor_t* visitor, block_t* block) {}

void walk_fn_decl(ast_visitor_t* visitor, fn_decl_t* decl) {
    visitor->visit_block(visitor, decl->block);
}
void walk_global_decl(ast_visitor_t* visitor, string_t* ident) {}
void walk_import_decl(ast_visitor_t* visitor, string_t* str) {}
void walk_decl(ast_visitor_t* visitor, decl_t* decl) {
    visitor->visit_decl_pre(visitor, decl);
    switch (decl->kind) {
        case DECL_FUNCTION: {
            visitor->visit_fn_decl(visitor, &decl->value.fn);
            break;
        }
        case DECL_GLOBAL: {
            visitor->visit_global_decl(visitor, &decl->value.str);
            break;
        }
        case DECL_IMPORT: {
            visitor->visit_import_decl(visitor, &decl->value.str);
            break;
        }
    }
    visitor->visit_decl_post(visitor, decl);
}
void walk_decl_pre(ast_visitor_t* visitor, decl_t* decl) {}
void walk_decl_post(ast_visitor_t* visitor, decl_t* decl) {}

void walk_program(ast_visitor_t* visitor, program_t* program) {
    visitor->visit_program_pre(visitor, program);
    for (int i = 0;  i < program->len; i++) {
        visitor->visit_decl(visitor, program->decls[i]);
    }
    visitor->visit_program_post(visitor, program);
}
void walk_program_pre(ast_visitor_t* visitor, program_t* program) {}
void walk_program_post(ast_visitor_t* visitor, program_t* program) {}