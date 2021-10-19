#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t len;
    // const char[len]
    const char* s;
} string_t;

void print_string(string_t s);

/* === EXPRESSIONS === */

struct expr;
typedef struct expr expr_t;

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
    uint32_t arity;
    // expr_t*[arity]
    expr_t** args;
} call_expr_t;

typedef struct {
    expr_t* array;
    expr_t* index;
} index_expr_t;

typedef struct {
    uint32_t len;
    // expr_t*[len]
    expr_t** exprs;
} array_expr_t;

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
        EXPR_ARRAY
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
        array_expr_t array;
    } value;
};

/* === STATEMENTS === */

struct block;
typedef struct block block_t;

typedef struct {
    expr_t* main_cond;
    block_t* main_block;
    uint32_t elif_len;
    // nullable if elif_len == 0. expr_t*[elif_len
    expr_t** elif_conds;
    // nullable if elif_len == 0. block_t*[elif_len]
    block_t** elif_blocks;
    // nullable.
    block_t* else_block;
} if_stmt_t;

typedef struct {
    expr_t* cond;
    block_t* block;
} while_stmt_t;

typedef struct {
    string_t ident;
    expr_t* value;
} assign_normal_t;

typedef struct {
    expr_t* array;
    expr_t* index;
    expr_t* value;
} assign_array_t;

typedef struct {
    enum {
        STMT_IF,
        STMT_WHILE,
        STMT_RETURN,
        STMT_ASSIGN_NORMAL,
        STMT_ASSIGN_ARRAY,
        STMT_DO
    } kind;
    union {
        // STMT_IF
        if_stmt_t if_stmt;
        // STMT_WHILE
        while_stmt_t while_stmt;
        // STMT_RETURN, STMT_DO
        expr_t* expr;
        // STMT_ASSIGN_NORMAL
        assign_normal_t assign_normal;
        // STMT_ASSIGN_ARRAY
        assign_array_t assign_array;
    } value;
} stmt_t;

struct block {
    uint32_t len;
    // stmt_t*[len]
    stmt_t** stmts;
};

/* === DECLARATIONS === */

typedef struct {
    string_t name;
    uint32_t arity;
    // string_t[arity]
    string_t* args;
    block_t* block;
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
        string_t str;
    } value;
} decl_t;

typedef struct {
    uint32_t len;
    // decl_t*[len]
    decl_t** decls;
} program_t;

// Make a string from a char* with a given length.
string_t mk_string(const char* s, uint32_t len);
// Make a string from a null-terminated C-string
string_t mk_string_cstr(const char* s);

/* === Make various expression types === */

expr_t* mk_binop(expr_t* lhs, expr_t* rhs, binop_t op);
expr_t* mk_unop(expr_t* subexpr, unop_t op);
expr_t* mk_call(expr_t* func, uint32_t arity, expr_t** args);
expr_t* mk_index(expr_t* array, expr_t* index);
expr_t* mk_array(uint32_t len, expr_t** exprs);
expr_t* mk_bool(bool val);
expr_t* mk_number(uint64_t number);
expr_t* mk_string_expr(string_t string);
expr_t* mk_ident(string_t ident);

/* === Make various statement types === */

stmt_t* mk_if(
    expr_t* main_cond,
    block_t* main_block,
    uint32_t elif_len,
    // nullable if elif_len == 0. expr_t*[elif_len]
    expr_t** elif_conds,
    // nullable if elif_len == 0. block_t*[elif_len]
    block_t** elif_blocks,
    // nullable.
    block_t* else_block
);
stmt_t* mk_while(expr_t* cond, block_t* block);
stmt_t* mk_return(expr_t* expr);
stmt_t* mk_assign_normal(string_t ident, expr_t* value);
stmt_t* mk_assign_array(expr_t* array, expr_t* index, expr_t* value);
stmt_t* mk_do(expr_t* expr);

// Make a block.
block_t* mk_block(uint32_t len, stmt_t** stmts);

/* === Make various declaration types === */

decl_t* mk_fn_decl(string_t name, uint32_t arity, string_t* args, block_t* block);
decl_t* mk_global(string_t ident);
decl_t* mk_import(string_t str);

// Make a program.
program_t* mk_program(uint32_t len, decl_t** decls);

// Box an array by copying it onto the heap.

expr_t** box_exprs(uint32_t len, expr_t** exprs);
stmt_t** box_stmts(uint32_t len, stmt_t** stmts);
block_t** box_blocks(uint32_t len, block_t** blocks);
decl_t** box_decls(uint32_t len, decl_t** decls);
string_t* box_strings(uint32_t len, string_t* strings);

// Free things.
// Note: free_plurals(len, array) WILL free() the array.

// void free_expr(expr_t* expr);
// void free_exprs(uint32_t len, expr_t** exprs);

// void free_stmt(stmt_t* stmt);
// void free_stmts(uint32_t len, stmt_t** stmts);

// void free_block(block_t* block);
// void free_blocks(uint32_t len, block_t** blocks);

// void free_decl(decl_t* decl);
// void free_decls(uint32_t len, decl_t** decls);

void free_program(program_t* program);