#include "ast.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void print_string(string_t str) {
    printf("%2$.*1$s", str.len, str.s);
}

string_t mk_string(const char* s, uint32_t len) {
    return (string_t) {
        .len = len,
        .s = s
    };
}

string_t mk_string_cstr(const char* s) {
    return mk_string(s, strlen(s));
}

expr_t* alloc_expr() {
    expr_t* expr = malloc(sizeof(expr_t));
    memset(expr, 0, sizeof(expr_t));
    return expr;
}

expr_t* mk_binop(expr_t* lhs, expr_t* rhs, binop_t op) {
    expr_t* expr = alloc_expr();
    expr->kind = EXPR_BINARY;
    expr->value.binary = (binop_expr_t) {
        .lhs = lhs,
        .rhs = rhs,
        .op = op
    };
    return expr;
}

expr_t* mk_unop(expr_t* subexpr, unop_t op) {
    expr_t* expr = alloc_expr();
    expr->kind = EXPR_UNARY;
    expr->value.unary = (unop_expr_t) {
        .subexpr = subexpr,
        .op = op
    };
    return expr;
}

expr_t* mk_call(expr_t* func, uint32_t arity, expr_t** args) {
    expr_t* expr = alloc_expr();
    expr->kind = EXPR_CALL;
    expr->value.call = (call_expr_t) {
        .func = func,
        .arity = arity,
        .args = args
    };
    return expr;
}

expr_t* mk_index(expr_t* array, expr_t* index) {
    expr_t* expr = alloc_expr();
    expr->kind = EXPR_INDEX;
    expr->value.index = (index_expr_t) {
        .array = array,
        .index = index
    };
    return expr;
}

expr_t* mk_array(uint32_t len, expr_t** exprs) {
    expr_t* expr = alloc_expr();
    expr->kind = EXPR_ARRAY;
    expr->value.array = (array_expr_t) {
        .len = len,
        .exprs = exprs
    };
    return expr;
}

expr_t* mk_bool(bool val) {
    expr_t* expr = alloc_expr();
    expr->kind = EXPR_BOOL;
    expr->value.bool_expr = val;
    return expr;
}

expr_t* mk_number(uint64_t number) {
    expr_t* expr = alloc_expr();
    expr->kind = EXPR_NUMBER;
    expr->value.number = number;
    return expr;
}

expr_t* mk_string_expr(string_t string) {
    expr_t* expr = alloc_expr();
    expr->kind = EXPR_STRING;
    expr->value.string = string;
    return expr;
}

expr_t* mk_ident(string_t ident) {
    expr_t* expr = alloc_expr();
    expr->kind = EXPR_IDENT;
    expr->value.string = ident;
    return expr;
}

stmt_t* alloc_stmt() {
    stmt_t* stmt = malloc(sizeof(stmt_t));
    memset(stmt, 0, sizeof(stmt_t));
    return stmt;
}

stmt_t* mk_if(
    expr_t* main_cond,
    block_t* main_block,
    uint32_t elif_len,
    expr_t** elif_conds,
    block_t** elif_blocks,
    block_t* else_block
) {
    stmt_t* stmt = alloc_stmt();
    stmt->kind = STMT_IF;
    stmt->value.if_stmt = (if_stmt_t) {
        .main_cond = main_cond,
        .main_block = main_block,
        .elif_len = elif_len,
        .elif_conds = elif_conds,
        .elif_blocks = elif_blocks,
        .else_block = else_block
    };
    return stmt;
}

stmt_t* mk_while(expr_t* cond, block_t* block) {
    stmt_t* stmt = alloc_stmt();
    stmt->kind = STMT_WHILE;
    stmt->value.while_stmt = (while_stmt_t) {
        .cond = cond,
        .block = block
    };
    return stmt;
}

stmt_t* mk_return(expr_t* expr) {
    stmt_t* stmt = alloc_stmt();
    stmt->kind = STMT_RETURN;
    stmt->value.expr = expr;
    return stmt;
}

stmt_t* mk_assign_normal(string_t ident, expr_t* value) {
    stmt_t* stmt = alloc_stmt();
    stmt->kind = STMT_ASSIGN_NORMAL;
    stmt->value.assign_normal = (assign_normal_t) {
        .ident = ident,
        .value = value
    };
    return stmt;
}

stmt_t* mk_assign_array(expr_t* array, expr_t* index, expr_t* value) {
    stmt_t* stmt = alloc_stmt();
    stmt->kind = STMT_ASSIGN_ARRAY;
    stmt->value.assign_array = (assign_array_t) {
        .array = array,
        .index = index,
        .value = value
    };
    return stmt;
}

stmt_t* mk_do(expr_t* expr) {
    stmt_t* stmt = alloc_stmt();
    stmt->kind = STMT_DO;
    stmt->value.expr = expr;
    return stmt;
}

block_t* mk_block(uint32_t len, stmt_t** stmts) {
    block_t* block = malloc(sizeof(block_t));
    memset(block, 0, sizeof(block_t));
    block->len = len;
    block->stmts = stmts;
    return block;
}

decl_t* alloc_decl() {
    decl_t* decl = malloc(sizeof(decl_t));
    memset(decl, 0, sizeof(decl_t));
    return decl;
}

decl_t* mk_fn_decl(string_t name, uint32_t arity, string_t* args, block_t* block) {
    decl_t* decl = alloc_decl();
    decl->kind = DECL_FUNCTION;
    decl->value.fn = (fn_decl_t) {
        .name = name,
        .arity = arity,
        .args = args,
        .block = block
    };
    return decl;
}

decl_t* mk_global(string_t ident) {
    decl_t* decl = alloc_decl();
    decl->kind = DECL_GLOBAL;
    decl->value.str = ident;
    return decl;
}

decl_t* mk_import(string_t str) {
    decl_t* decl = alloc_decl();
    decl->kind = DECL_IMPORT;
    decl->value.str = str;
    return decl;
}

program_t* mk_program(uint32_t len, decl_t** decls) {
    program_t* program = malloc(sizeof(program_t));
    memset(program, 0, sizeof(program_t));
    program->len = len;
    program->decls = decls;
    return program;
}

#define BOX_IMPL(ty, plural) \
    ty* box_##plural(uint32_t len, ty* plural) { \
        ty* new_##plural = malloc(sizeof(ty) * len); \
        memcpy(new_##plural, plural, sizeof(ty) * len); \
        return new_##plural; \
    }

BOX_IMPL(expr_t*, exprs)
BOX_IMPL(stmt_t*, stmts)
BOX_IMPL(block_t*, blocks)
BOX_IMPL(decl_t*, decls)
BOX_IMPL(string_t, strings)