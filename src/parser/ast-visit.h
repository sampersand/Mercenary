#pragma once

#include "ast.h"
#include <stdlib.h>

#define FN(ident, args) void (*ident)(ast_visitor_t*, args)
#define FN_(ident) void (*ident)(ast_visitor_t*)

struct ast_visitor;
typedef struct ast_visitor ast_visitor_t;

struct ast_visitor {
    void* data;

    FN(visit_binop_expr, binop_expr_t*);
    FN(visit_unop_expr, unop_expr_t*);
    FN(visit_call_expr, call_expr_t*);
    FN(visit_index_expr, index_expr_t*);
    FN(visit_bool_expr, bool*);
    FN(visit_number_expr, uint64_t*);
    FN(visit_string_expr, string_t*);
    FN(visit_ident_expr, string_t*);
    FN(visit_array_expr, expr_array_t*);
    FN_(visit_null_expr);
    FN(visit_expr, expr_t*);
    FN(visit_expr_pre, expr_t*);
    FN(visit_expr_post, expr_t*);

    FN(visit_if_stmt, if_stmt_t*);
    FN(visit_while_stmt, while_stmt_t*);
    FN(visit_return_stmt, expr_t*);
    FN(visit_do_stmt, expr_t*);
    FN(visit_assign_normal, assign_normal_t*);
    FN(visit_assign_array, assign_array_t*);
    FN(visit_global_assign_normal, assign_normal_t*);
    FN(visit_global_assign_array, assign_array_t*);
    FN(visit_stmt, stmt_t*);
    FN(visit_stmt_pre, stmt_t*);
    FN(visit_stmt_post, stmt_t*);

    FN(visit_block, block_t*);
    FN(visit_block_pre, block_t*);
    FN(visit_block_post, block_t*);

    FN(visit_fn_decl, fn_decl_t*);
    FN(visit_global_decl, string_t*);
    FN(visit_import_decl, string_t*);
    FN(visit_decl, decl_t*);
    FN(visit_decl_pre, decl_t*);
    FN(visit_decl_post, decl_t*);

    FN(visit_program, program_t*);
    FN(visit_program_pre, program_t*);
    FN(visit_program_post, program_t*);
};

void walk_binop_expr(ast_visitor_t* visitor, binop_expr_t* expr);
void walk_unop_expr(ast_visitor_t* visitor, unop_expr_t* expr);
void walk_call_expr(ast_visitor_t* visitor, call_expr_t* expr);
void walk_index_expr(ast_visitor_t* visitor, index_expr_t* expr);
void walk_bool_expr(ast_visitor_t* visitor, bool* expr);
void walk_number_expr(ast_visitor_t* visitor, uint64_t* expr);
void walk_string_expr(ast_visitor_t* visitor, string_t* expr);
void walk_ident_expr(ast_visitor_t* visitor, string_t* expr);
void walk_array_expr(ast_visitor_t* visitor, expr_array_t* expr);
void walk_null_expr(ast_visitor_t* visitor);
void walk_expr(ast_visitor_t* visitor, expr_t* expr);
void walk_expr_pre(ast_visitor_t* visitor, expr_t* expr);
void walk_expr_post(ast_visitor_t* visitor, expr_t* expr);

void walk_if_stmt(ast_visitor_t* visitor, if_stmt_t* stmt);
void walk_while_stmt(ast_visitor_t* visitor, while_stmt_t* stmt);
void walk_return_stmt(ast_visitor_t* visitor, expr_t* stmt);
void walk_do_stmt(ast_visitor_t* visitor, expr_t* stmt);
void walk_assign_normal(ast_visitor_t* visitor, assign_normal_t* stmt);
void walk_assign_array(ast_visitor_t* visitor, assign_array_t* stmt);
void walk_global_assign_normal(ast_visitor_t* visitor, assign_normal_t* stmt);
void walk_global_assign_array(ast_visitor_t* visitor, assign_array_t* stmt);
void walk_stmt(ast_visitor_t* visitor, stmt_t* stmt);
void walk_stmt_pre(ast_visitor_t* visitor, stmt_t* stmt);
void walk_stmt_post(ast_visitor_t* visitor, stmt_t* stmt);

void walk_block(ast_visitor_t* visitor, block_t* block);
void walk_block_pre(ast_visitor_t* visitor, block_t* block);
void walk_block_post(ast_visitor_t* visitor, block_t* block);

void walk_fn_decl(ast_visitor_t* visitor, fn_decl_t* decl);
void walk_global_decl(ast_visitor_t* visitor, string_t* ident);
void walk_import_decl(ast_visitor_t* visitor, string_t* str);
void walk_decl(ast_visitor_t* visitor, decl_t* decl);
void walk_decl_pre(ast_visitor_t* visitor, decl_t* decl);
void walk_decl_post(ast_visitor_t* visitor, decl_t* decl);

void walk_program(ast_visitor_t* visitor, program_t* program);
void walk_program_pre(ast_visitor_t* visitor, program_t* program);
void walk_program_post(ast_visitor_t* visitor, program_t* program);

static ast_visitor_t DEFAULT_VISITOR = {
    .data = NULL,

    .visit_binop_expr = walk_binop_expr,
    .visit_unop_expr = walk_unop_expr,
    .visit_call_expr = walk_call_expr,
    .visit_index_expr = walk_index_expr,
    .visit_bool_expr = walk_bool_expr,
    .visit_number_expr = walk_number_expr,
    .visit_string_expr = walk_string_expr,
    .visit_ident_expr = walk_ident_expr,
    .visit_array_expr = walk_array_expr,
    .visit_null_expr = walk_null_expr,
    .visit_expr = walk_expr,
    .visit_expr_pre = walk_expr_pre,
    .visit_expr_post = walk_expr_post,

    .visit_if_stmt = walk_if_stmt,
    .visit_while_stmt = walk_while_stmt,
    .visit_return_stmt = walk_return_stmt,
    .visit_do_stmt = walk_do_stmt,
    .visit_assign_normal = walk_assign_normal,
    .visit_assign_array = walk_assign_array,
    .visit_global_assign_normal = walk_global_assign_normal,
    .visit_global_assign_array = walk_global_assign_array,
    .visit_stmt = walk_stmt,
    .visit_stmt_pre = walk_stmt_pre,
    .visit_stmt_post = walk_stmt_post,

    .visit_block = walk_block,
    .visit_block_pre = walk_block_pre,
    .visit_block_post = walk_block_post,

    .visit_fn_decl = walk_fn_decl,
    .visit_global_decl = walk_global_decl,
    .visit_import_decl = walk_import_decl,
    .visit_decl = walk_decl,
    .visit_decl_pre = walk_decl_pre,
    .visit_decl_post = walk_decl_post,

    .visit_program = walk_program,
    .visit_program_pre = walk_program_pre,
    .visit_program_post = walk_program_post
};