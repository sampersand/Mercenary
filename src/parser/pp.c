#include "pp.h"

#define PP_DATA(visitor) ((pp_data*)visitor->data)

typedef struct {
    bool space_before_decl;
} pp_data;

void pp_program_pre(ast_visitor_t* visitor, program_t* program) {
    printf("(program");
    if (arr_get_size(*program) > 0) {
        PP_DATA(visitor)->space_before_decl = true;
    }
}

void pp_program_post(ast_visitor_t* visitor, program_t* program) {
    printf(")\n");
    PP_DATA(visitor)->space_before_decl = false;
}

void pp_decl_pre(ast_visitor_t* visitor, decl_t* decl) {
    if (PP_DATA(visitor)->space_before_decl) {
        printf(" ");
    }
    printf("(decl ");
}

void pp_decl_post(ast_visitor_t* visitor, decl_t* decl) {
    printf(")");
}

void pp_fn_decl(ast_visitor_t* visitor, fn_decl_t* decl) {
    printf("(fn ");
    print_string(decl->name);
    printf(" (args");
    for (int i = 0; i < arr_get_size(decl->args); i++) {
        printf(" ");
        print_string(arr_at(decl->args, i));
    }
    printf(") ");
    visitor->visit_block(visitor, &decl->block);
}

void pp_global_decl(ast_visitor_t* visitor, string_t* ident) {
    printf("(global ");
    print_string(*ident);
    printf(")");
}

void pp_import_decl(ast_visitor_t* visitor, string_t* str) {
    printf("(import ");
    print_string(*str);
    printf(")");
}

void pp_block(ast_visitor_t* visitor, block_t* block) {
    printf("(block");
    for (int i = 0; i < arr_get_size(*block); i++) {
        printf(" ");
        visitor->visit_stmt(visitor, &arr_at(*block, i));
    }
    printf(")");
}

void pp_stmt_pre(ast_visitor_t* visitor, stmt_t* stmt) {
    printf("(stmt ");
}

void pp_stmt_post(ast_visitor_t* visitor, stmt_t* stmt) {
    printf(")");
}

void pp_if_stmt(ast_visitor_t* visitor, if_stmt_t* stmt) {
    printf("(if (expr ");
    visitor->visit_expr(visitor, stmt->main_cond);
    printf(") ");
    visitor->visit_block(visitor, &stmt->main_block);
    for (int i = 0; i < arr_get_size(stmt->elif_conds); i++) {
        printf(" (elif (expr ");
        visitor->visit_expr(visitor, &arr_at(stmt->elif_conds, i));
        printf(") ");
        visitor->visit_block(visitor, &arr_at(stmt->elif_blocks, i));
        printf(")");
    }
    if (stmt->else_block != NULL) {
        printf(" (else ");
        visitor->visit_block(visitor, &stmt->else_block);
        printf(")");
    }
    printf(")");
}

void pp_while_stmt(ast_visitor_t* visitor, while_stmt_t* stmt) {
    printf("(while (expr ");
    visitor->visit_expr(visitor, stmt->cond);
    printf(") ");
    visitor->visit_block(visitor, &stmt->block);
    printf(")");
}

void pp_return_stmt(ast_visitor_t* visitor, expr_t* expr) {
    printf("(return (expr ");
    visitor->visit_expr(visitor, expr);
    printf("))");
}

void pp_do_stmt(ast_visitor_t* visitor, expr_t* expr) {
    printf("(do ");
    visitor->visit_expr(visitor, expr);
    printf(")");
}

void pp_declare_var(ast_visitor_t* visitor, declare_var_t* stmt) {
    printf("(decl-var ");
    print_string(stmt->ident);
    printf(" (expr ");
    visitor->visit_expr(visitor, stmt->value);
    printf("))");
}

void pp_assign_var(ast_visitor_t* visitor, assign_var_t* stmt) {
    printf("(assign ");
    print_string(stmt->ident);
    if (arr_get_size(stmt->indices) > 0) {
        printf(" (indices");
        for (int i = 0; i < arr_get_size(stmt->indices); i++) {
            printf(" (expr ");
            visitor->visit_expr(visitor, &arr_at(stmt->indices, i));
            printf(")");
        }
        printf(") ");
    }
    printf("(expr ");
    visitor->visit_expr(visitor, stmt->value);
    printf("))");
}

void pp_binop_expr(ast_visitor_t* visitor, binop_expr_t* expr) {
    const char* op;
    switch (expr->op) {
        case BINOP_EQ:  op = "=="; break;
        case BINOP_NE:  op = "!="; break;
        case BINOP_GT:  op = ">"; break;
        case BINOP_GE:  op = ">="; break;
        case BINOP_LT:  op = "<"; break;
        case BINOP_LE:  op = "<="; break;
        case BINOP_AND: op = "&"; break;
        case BINOP_OR:  op = "|"; break;
        case BINOP_ADD: op = "+"; break;
        case BINOP_SUB: op = "-"; break;
        case BINOP_MUL: op = "*"; break;
        case BINOP_DIV: op = "/"; break;
        case BINOP_REM: op = "%"; break;
        default:        op = "???"; break;
    }
    printf("(%s ", op);
    visitor->visit_expr(visitor, expr->lhs);
    printf(" ");
    visitor->visit_expr(visitor, expr->rhs);
    printf(")");
}

void pp_unop_expr(ast_visitor_t* visitor, unop_expr_t* expr) {
    const char* op;
    switch (expr->op) {
        case UNOP_NEGATE: op = "-"; break;
        case UNOP_NOT:    op = "!"; break;
        default:          op = "???"; break;
    }
    printf("(%s ", op);
    visitor->visit_expr(visitor, expr->subexpr);
    printf(")");
}

void pp_call_expr(ast_visitor_t* visitor, call_expr_t* expr) {
    printf("(call ");
    visitor->visit_expr(visitor, expr->func);
    printf(" (args");
    for (int i = 0; i < arr_get_size(expr->args); i++) {
        printf(" ");
        visitor->visit_expr(visitor, &arr_at(expr->args, i));
    }
    printf("))");
}

void pp_index_expr(ast_visitor_t* visitor, index_expr_t* expr) {
    printf("(index ");
    visitor->visit_expr(visitor, expr->array);
    printf(" ");
    visitor->visit_expr(visitor, expr->index);
    printf(")");
}

void pp_bool_expr(ast_visitor_t* visitor, bool* expr) {
    printf("%s", expr ? "true" : "false");
}

void pp_null_expr(ast_visitor_t* visitor) {
    printf("null");
}

void pp_number_expr(ast_visitor_t* visitor, uint64_t* expr) {
    printf("%lu", *expr);
}

void pp_string_expr(ast_visitor_t* visitor, string_t* expr) {
    print_string(*expr);
}

void pp_ident_expr(ast_visitor_t* visitor, string_t* expr) {
    print_string(*expr);
}

void pp_array_expr(ast_visitor_t* visitor, expr_array_t* expr) {
    printf("(array");
    for (int i = 0; i < arr_get_size(*expr); i++) {
        printf(" ");
        visitor->visit_expr(visitor, &arr_at(*expr, i));
    }
    printf(")");
}

ast_visitor_t pp_visitor() {
    ast_visitor_t visitor = DEFAULT_VISITOR;
    
    visitor.data = malloc(sizeof(pp_data));
    ((pp_data*) visitor.data)->space_before_decl = false;

    visitor.visit_program_pre = pp_program_pre;
    visitor.visit_program_post = pp_program_post;
    visitor.visit_decl_pre = pp_decl_pre;
    visitor.visit_decl_post = pp_decl_post;
    visitor.visit_fn_decl = pp_fn_decl;
    visitor.visit_global_decl = pp_global_decl;
    visitor.visit_import_decl = pp_import_decl;
    visitor.visit_block = pp_block;
    visitor.visit_stmt_pre = pp_stmt_pre;
    visitor.visit_stmt_post = pp_stmt_post;
    visitor.visit_if_stmt = pp_if_stmt;
    visitor.visit_while_stmt = pp_while_stmt;
    visitor.visit_return_stmt = pp_return_stmt;
    visitor.visit_do_stmt = pp_do_stmt;
    visitor.visit_declare_var = pp_declare_var;
    visitor.visit_assign_var = pp_assign_var;
    visitor.visit_binop_expr = pp_binop_expr;
    visitor.visit_unop_expr = pp_unop_expr;
    visitor.visit_call_expr = pp_call_expr;
    visitor.visit_index_expr = pp_index_expr;
    visitor.visit_bool_expr = pp_bool_expr;
    visitor.visit_null_expr = pp_null_expr;
    visitor.visit_number_expr = pp_number_expr;
    visitor.visit_string_expr = pp_string_expr;
    visitor.visit_ident_expr = pp_ident_expr;
    visitor.visit_array_expr = pp_array_expr;
    return visitor;
}

void pp_program(program_t program) {
    ast_visitor_t visitor = pp_visitor();
    visitor.visit_program(&visitor, &program);
    free((pp_data*)visitor.data);
}