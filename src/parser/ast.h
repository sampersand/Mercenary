#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "dyn_array.h"

// TODO: use dyn_array.h for lists inside AST

typedef struct {
    uint32_t len;
    // const char[len]
    const char* s;
} string_t;

arr_forward_decl(string_array_t);
arr_decl(string_array_t, string_t);

void print_string(string_t s);

/* === EXPRESSIONS === */

struct expr;
typedef struct expr expr_t;

arr_forward_decl(expr_array_t)

typedef enum {
    BINOP_EQ,
    BINOP_NE,
    BINOP_GT,
    BINOP_GE,
    BINOP_LT,
    BINOP_LE,
    BINOP_AND,
    BINOP_OR,
    BINOP_ADD,
    BINOP_SUB,
    BINOP_MUL,
    BINOP_DIV,
    BINOP_REM
} binop_t;

typedef struct {
    expr_t* lhs;
    expr_t* rhs;
    binop_t op;
} binop_expr_t;

typedef enum {
    UNOP_NEGATE,
    UNOP_NOT
} unop_t;

typedef struct {
    expr_t* subexpr;
    unop_t op;
} unop_expr_t;

typedef struct {
    expr_t* func;
    expr_array_t args;
} call_expr_t;

typedef struct {
    expr_t* array;
    expr_t* index;
} index_expr_t;

struct expr {
    enum {
        EXPR_BINARY,
        EXPR_UNARY,
        EXPR_CALL,
        EXPR_INDEX,
        EXPR_BOOL,
        EXPR_NUMBER,
        EXPR_STRING,
        EXPR_IDENT,
        EXPR_ARRAY,
        EXPR_NULL,
    } kind;
    union {
        // EXPR_BINARY
        binop_expr_t binary;
        // EXPR_UNARY
        unop_expr_t unary;
        // EXPR_CALL
        call_expr_t call;
        // EXPR_INDEX
        index_expr_t index;
        // EXPR_BOOL
        bool bool_expr;
        // EXPR_NUMBER
        uint64_t number;
        // EXPR_STRING, EXPR_IDENT
        string_t string;
        // EXPR_ARRAY
        expr_array_t array;
    } value;
};

arr_decl(expr_array_t, expr_t)

/* === STATEMENTS === */

arr_forward_decl(stmt_array_t)

typedef stmt_array_t block_t;

arr_forward_decl(block_array_t)

typedef struct {
    expr_t* main_cond;
    block_t main_block;
    // length must be same of elif_conds and elif_blocks
    // expr_t[elif_len]
    expr_array_t elif_conds;
    // block_t[elif_len]
    block_array_t elif_blocks;
    // nullable.
    block_t* else_block;
} if_stmt_t;

typedef struct {
    expr_t* cond;
    block_t block;
} while_stmt_t;

typedef struct {
    string_t ident;
    expr_t* value;
} declare_var_t;

typedef struct {
    string_t ident;
    expr_array_t indices;
    expr_t* value;
} assign_var_t;

typedef struct {
    enum {
        STMT_IF,
        STMT_WHILE,
        STMT_RETURN,
        STMT_DECLARE_VAR,
        STMT_ASSIGN_VAR,
        STMT_DO
    } kind;
    union {
        // STMT_IF
        if_stmt_t if_stmt;
        // STMT_WHILE
        while_stmt_t while_stmt;
        // STMT_RETURN, STMT_DO
        expr_t* expr;
        // STMT_DECLARE_VAR
        declare_var_t declare_var;
        // STMT_ASSIGN_VAR
        assign_var_t assign_var;
    } value;
} stmt_t;

arr_decl(stmt_array_t, stmt_t)
arr_decl(block_array_t, block_t)

/* === DECLARATIONS === */

typedef struct {
    string_t name;
    string_array_t args;
    block_t block;
} fn_decl_t;

typedef struct {
    enum {
        DECL_FUNCTION,
        DECL_GLOBAL,
        DECL_IMPORT
    } kind;
    union {
        // DECL_FUNCTION
        fn_decl_t fn;
        // DECL_GLOBAL, DECL_IMPORT
        string_t string;
    } value;
} decl_t;

arr_forward_decl(decl_array_t)
arr_decl(decl_array_t, decl_t)

typedef decl_array_t program_t;

// Make a string from a char* with a given length.
string_t mk_string(const char* s, uint32_t len);
// Make a string from a null-terminated C-string.
string_t mk_string_cstr(const char* s);
// Make a string from a start and end pointer.
string_t mk_string_2ptrs(const char* start, const char* end);

/* === Make various expression types === */

expr_t mk_binop(expr_t lhs, expr_t rhs, binop_t op);
expr_t mk_unop(expr_t subexpr, unop_t op);
expr_t mk_call(expr_t func, expr_array_t args);
expr_t mk_index(expr_t array, expr_t index);
expr_t mk_array(expr_array_t exprs);
expr_t mk_bool(bool val);
expr_t mk_number(uint64_t number);
expr_t mk_null();
expr_t mk_string_expr(string_t string);
expr_t mk_ident(string_t ident);

/* === Make various statement types === */

stmt_t mk_if(
    expr_t main_cond,
    block_t main_block,
    // length must be same of elif_conds and elif_blocks
    // expr_t[elif_len]
    expr_array_t elif_conds,
    // block_t[elif_len]
    block_array_t elif_blocks,
    // nullable.
    block_t* else_block
);
stmt_t mk_while(expr_t cond, block_t block);
stmt_t mk_return(expr_t expr);
stmt_t mk_declare_var(string_t ident, expr_t value);
stmt_t mk_assign_var(string_t ident, expr_array_t indexes, expr_t value);
stmt_t mk_do(expr_t expr);

// Make a block.
block_t mk_block(stmt_array_t stmts);

/* === Make various declaration types === */

decl_t mk_fn_decl(string_t name, string_array_t args, block_t block);
decl_t mk_global(string_t ident);
decl_t mk_import(string_t string);

// Free a program. Only use if you want to free the entire AST,
// as this will do so.
void free_program(program_t program);

// Free a block.
void free_block(block_t block);

// Free an expression
void free_expr(expr_t expr);