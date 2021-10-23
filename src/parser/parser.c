#include "parser.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "dyn_array.h"

// TODO proper error handling lmao

pres_t expect(const char** stream, uint64_t type, const char* error, Token* tok) {
    Token t = read_token(*stream);
    *stream = t.end;

    if (t.type == type) {
        if (tok != NULL) {
            *tok = t;
        }
        return PARSE_OK;
    } else {
        fprintf(stderr, "Syntax error: %s.\n", error);
        return PARSE_BAD;
    }
}

pres_t parse_program(const char** stream, program_t* program) {
    arr_alloc(*program);

    for (;;) {
        decl_t decl;
        switch (parse_decl(stream, &decl)) {
            case PARSE_OK: {
                arr_append(*program) = decl;
                break;
            }
            case PARSE_BAD: {
                free_program(*program);
                return PARSE_BAD;
            }
            default: return PARSE_OK;
        }
    }

    return PARSE_OK;
}

pres_t parse_decl(const char** stream, decl_t* decl) {
    Token t = read_token(*stream);
    *stream = t.end;

    switch (t.type) {
        case TOKEN_ERROR: {
            fputs("Syntax error: invalid token.\n", stderr);
            return PARSE_BAD;
        }
        // EOF
        case 0: {
            return PARSE_OK_NOTHING;
        }
        case TOKEN_IMPORT: {
            Token string;
            if (!expect(stream, TOKEN_STRING, "expected string", &string) ||
                !expect(stream, ';', "expected semicolon", NULL)) {
                return PARSE_BAD;
            }
            *decl = mk_import(mk_string_2ptrs(string.start, string.end));
            break;
        }
        case TOKEN_GLOBAL: {
            Token ident;
            if (!expect(stream, TOKEN_IDENTIFIER, "expected identifier", &ident) ||
                !expect(stream, ';', "expected semicolon", NULL)) {
                return PARSE_BAD;
            }
            *decl = mk_global(mk_string_2ptrs(ident.start, ident.end));
            break;
        }
        case TOKEN_FUNCTION: {
            Token name;
            if (!expect(stream, TOKEN_IDENTIFIER, "expected identifier", &name) ||
                !expect(stream, '(', "expected opening paren", NULL)) {
                return PARSE_BAD;
            }
            
            string_array_t args;
            arr_alloc(args);
            bool exit = false;
            bool is_first = true;
            while (!exit) {
                Token t = read_token(*stream);
                *stream = t.end;
                switch (t.type) {
                    case TOKEN_ERROR: {
                        fputs("Syntax error: invalid token.\n", stderr);
                        arr_free(args);
                        return PARSE_BAD;
                    }
                    case TOKEN_IDENTIFIER: {
                        if (is_first) {
                            arr_append(args) = mk_string_2ptrs(t.start, t.end);
                        } else {
                            fputs("Syntax error: expected `)` or `,`.\n", stderr);
                            arr_free(args);
                            return PARSE_BAD;
                        }
                        break;
                    }
                    case ',': {
                        Token ident;
                        if (!expect(stream, TOKEN_IDENTIFIER, "expected ident", &ident)) {
                            arr_free(args);
                            return PARSE_BAD;
                        }
                        arr_append(args) = mk_string_2ptrs(ident.start, ident.end);
                        break;
                    }
                    case ')': {
                        exit = true;
                        break;
                    }
                    default: {
                        fputs("Syntax error: expected ident or `)`.\n", stderr);
                        arr_free(args);
                        return PARSE_BAD;
                    }
                }
                is_first = false;
            }
            block_t block;
            if (!parse_block(stream, &block)) {
                arr_free(args);
                return PARSE_BAD;
            }
            *decl = mk_fn_decl(mk_string_2ptrs(name.start, name.end), args, block);
            break;
        }
        default: {
            fputs("Syntax error: expected `import`, `global`, or `function`.\n", stderr);
            return PARSE_BAD;
        }
    }

    return PARSE_OK;
}

pres_t parse_block(const char** stream, block_t* block) {
    arr_alloc(*block);

    if (!expect(stream, '{', "expected `{`", NULL)) {
        free_block(*block);
        return PARSE_BAD;
    }

    bool cont = true;
    while (cont) {
        Token peek = read_token(*stream);

        if (peek.type == '}') {
            *stream = peek.end;
            return PARSE_OK;
        }

        stmt_t stmt;
        switch (parse_stmt(stream, &stmt)) {
            case PARSE_OK: {
                arr_append(*block) = stmt;
                break;
            }
            case PARSE_BAD: {
                free_block(*block);
                return PARSE_BAD;
            }
            default: {
                cont = false;
                break;
            }
        }
    }

    if (!expect(stream, '}', "expected `}`", NULL)) {
        free_block(*block);
        return PARSE_BAD;
    }

    return PARSE_OK;
}

pres_t parse_stmt(const char** stream, stmt_t* stmt) {
    Token t = read_token(*stream);
    *stream = t.end;

    switch (t.type) {
        case TOKEN_ERROR: {
            fputs("Syntax error: invalid token.\n", stderr);
            return PARSE_BAD;
        }
        // EOF
        case 0: {
            return PARSE_OK_NOTHING;
        }
        case TOKEN_RETURN: {
            expr_t expr;
            if (parse_expr(stream, &expr) != PARSE_OK) {
                return PARSE_BAD;
            }
            if (!expect(stream, ';', "expected `;`", NULL)) {
                free_expr(expr);
                return PARSE_BAD;
            }
            *stmt = mk_do(expr);
            break;
        }
        case TOKEN_DO: {
            expr_t expr;
            if (parse_expr(stream, &expr) != PARSE_OK) {
                return PARSE_BAD;
            }
            if (!expect(stream, ';', "expected `;`", NULL)) {
                free_expr(expr);
                return PARSE_BAD;
            }
            *stmt = mk_do(expr);
            break;
        }
        case TOKEN_LET: {
            Token ident;
            expr_t value;
            if (!expect(stream, TOKEN_IDENTIFIER, "expected ident", &ident) ||
                !expect(stream, '=', "expected `=`", NULL) ||
                parse_expr(stream, &value) != PARSE_OK) {
                    return PARSE_BAD;
            }
            if (!expect(stream, ';', "expected `;`", NULL)) {
                free_expr(value);
                return PARSE_BAD;
            }
            *stmt = mk_declare_var(mk_string_2ptrs(ident.start, ident.end), value);
            break;
        }
        case TOKEN_WHILE: {
            expr_t cond;
            block_t block;
            if (!expect(stream, '(', "expected `(`", NULL) ||
                parse_expr(stream, &cond) != PARSE_OK) {
                return PARSE_BAD;
            }
            if (!expect(stream, ')', "expected `)`", NULL) ||
                parse_block(stream, &block) != PARSE_OK) {
                free_expr(cond);
                return PARSE_BAD;
            }
            *stmt = mk_while(cond, block);
            return PARSE_OK;
        }
        case TOKEN_IF: {
            expr_t main_cond;
            block_t main_block;
            expr_array_t elif_conds;
            arr_alloc(elif_conds);
            block_array_t elif_blocks;
            arr_alloc(elif_blocks);
            block_t else_block = NULL;

            if (!expect(stream, '(', "expected `(`", NULL) ||
                parse_expr(stream, &main_cond) != PARSE_OK) {
                goto if_cleanup;
            }
            if (!expect(stream, ')', "expected `)`", NULL) ||
                parse_block(stream, &main_block) != PARSE_OK) {
                free_expr(main_cond);
                goto if_cleanup;
            }

            bool cont = true;
            while (cont) {
                Token peek = read_token(*stream);
                switch (peek.type) {
                    case TOKEN_ERROR: {
                        fputs("Syntax error: invalid token.\n", stderr);
                        goto if_cleanup_else_loop;
                    }
                    // EOF
                    case 0: {
                        cont = false;
                        break;
                    }
                    case TOKEN_ELSE: {
                        *stream = peek.end;
                        Token peek1 = read_token(*stream);
                        switch (peek1.type) {
                            case TOKEN_ERROR: {
                                fputs("Syntax error: invalid token.\n", stderr);
                                goto if_cleanup_else_loop;
                            }
                            // EOF
                            case 0: {
                                fputs("Syntax error: expected `if` or `{`.\n", stderr);
                                goto if_cleanup_else_loop;
                            }
                            case TOKEN_IF: {
                                *stream = peek1.end;
                                expr_t cond;
                                block_t block;
                                if (!expect(stream, '(', "expected `(`", NULL) ||
                                    parse_expr(stream, &cond) != PARSE_OK) {
                                    goto if_cleanup_else_loop;
                                }
                                if (!expect(stream, ')', "expected `)`", NULL) ||
                                    parse_block(stream, &block) != PARSE_OK) {
                                    free_expr(cond);
                                    goto if_cleanup_else_loop;
                                }
                                arr_append(elif_conds) = cond;
                                // NOLINTNEXTLINE(bugprone-sizeof-expression)
                                arr_append(elif_blocks) = block;
                                break;
                            }
                            default: {
                                if (parse_block(stream, &else_block) != PARSE_OK) {
                                    goto if_cleanup_else_loop;
                                }
                                cont = false;
                                break;
                            };
                        }
                        break;
                    }
                    default: {
                        cont = false;
                        break;
                    }
                }

                _: continue;
                if_cleanup_else_loop:
                    free_expr(main_cond);
                    free_block(main_block);
                    for (int i = 0; i < arr_get_size(elif_conds); i++) {
                        free_expr(arr_at(elif_conds, i));
                        free_block(arr_at(elif_blocks, i));
                    }
                    arr_free(elif_conds);
                    arr_free(elif_blocks);
                    return PARSE_BAD;
            }

            *stmt = mk_if(main_cond, main_block, elif_conds, elif_blocks, else_block);
            return PARSE_OK;

            if_cleanup:
                arr_free(elif_conds);
                arr_free(elif_blocks);
                return PARSE_BAD;
        }
        case TOKEN_SET: {
            Token ident;
            expr_array_t indices;
            arr_alloc(indices);
            expr_t value;

            if (!expect(stream, TOKEN_IDENTIFIER, "expected ident", &ident)) {
                arr_free(indices);
                return PARSE_BAD;
            }

            bool cont = true;
            while (cont) {
                Token tok = read_token(*stream);
                *stream = tok.end;

                switch (tok.type) {
                    case TOKEN_ERROR: {
                        fputs("Syntax error: invalid token.\n", stderr);
                        goto set_arrays_cleanup;
                    }
                    case '[': {
                        expr_t expr;
                        if (parse_expr(stream, &expr) != PARSE_OK) {
                            goto set_arrays_cleanup;
                        }
                        if (!expect(stream, ']', "expected `]`", NULL)) {
                            free_expr(expr);
                            goto set_arrays_cleanup;
                        }
                        arr_append(indices) = expr;
                        break;
                    }
                    case '=': {
                        if (parse_expr(stream, &value) != PARSE_OK) {
                            goto set_arrays_cleanup;
                        }
                        if (!expect(stream, ';', "expected `;`", NULL)) {
                            free_expr(value);
                            goto set_arrays_cleanup;
                        }
                        cont = false;
                        break;
                    }
                    default: {
                        fputs("Syntax error: expected `[` or `=`.\n", stderr);
                        goto set_arrays_cleanup;
                    }
                }

                __: continue;
                set_arrays_cleanup:
                    for (int i = 0; i < arr_get_size(indices); i++) {
                        free_expr(arr_at(indices, i));
                    }
                    arr_free(indices);
                    return PARSE_BAD;
            }

            *stmt = mk_assign_var(mk_string_2ptrs(ident.start, ident.end), indices, value);
            return PARSE_OK;
        }
        default: {
            fputs("Syntax error: expected `if`, `while`, `return`, `let`, `set`, or `do`.\n", stderr);
            return PARSE_BAD;
        }
    }


    return PARSE_OK;
}

pres_t parse_literal(const char** stream, expr_t* expr) {
    Token t = read_token(*stream);
    *stream = t.end;

    switch (t.type) {
        case TOKEN_ERROR: {
            fputs("Syntax error: invalid token.\n", stderr);
            return PARSE_BAD;
        }
        // EOF
        case 0: {
            return PARSE_OK_NOTHING;
        }
        case TOKEN_TRUE: {
            *expr = mk_bool(true);
            break;
        }
        case TOKEN_FALSE: {
            *expr = mk_bool(false);
            break;
        }
        case TOKEN_NULL: {
            *expr = mk_null();
            break;
        }
        case TOKEN_STRING: {
            *expr = mk_string_expr(mk_string_2ptrs(t.start, t.end));
            break;
        }
        case TOKEN_IDENTIFIER: {
            *expr = mk_ident(mk_string_2ptrs(t.start, t.end));
            break;
        }
        case TOKEN_NUMBER: {
            size_t len = t.end - t.start;
            char* num_s = malloc(len + 1);
            memcpy(num_s, t.start, len);
            num_s[len] = 0;
            char* inval = NULL;
            uint64_t num = strtoul(num_s, &inval, 10);
            if (*inval != 0) {
                fputs("Syntax error: invalid number literal.\n", stderr);
                free(num_s);
                return PARSE_BAD;
            } else if (errno == ERANGE) {
                fputs("Syntax error: number literal would overflow.\n", stderr);
                free(num_s);
                return PARSE_BAD;
            }
            free(num_s);
            *expr = mk_number(num);
            break;
        }
        case '[': {
            expr_array_t exprs;
            arr_alloc(exprs);

            bool cont = true;
            while (cont) {
                Token peek = read_token(*stream);

                switch (peek.type) {
                    case TOKEN_ERROR: {
                        fputs("Syntax error: invalid token.\n", stderr);
                        goto array_expr_loop_cleanup;
                    }
                    case ']': {
                        *stream = peek.end;
                        cont = false;
                        break;
                    }
                    case ',': {
                        *stream = peek.end;
                    }
                    default: {
                        expr_t expr;
                        if (parse_expr(stream, &expr) != PARSE_OK) {
                            goto array_expr_loop_cleanup;
                        }
                        arr_append(exprs) = expr;
                        break;
                    }
                }

                _: continue;
                array_expr_loop_cleanup:
                    for (int i = 0; i < arr_get_size(exprs); i++) {
                        free_expr(arr_at(exprs, i));
                    }
                    arr_free(exprs);
                    return PARSE_BAD;
            }
            *expr = mk_array(exprs);
            break;
        }

        default: {
            fputs("Syntax error: expected a literal expression or identifier.\n", stderr);
            return PARSE_BAD;
        }
    }

    return PARSE_OK;
}

pres_t parse_primary(const char** stream, expr_t* expr) {
    Token peek = read_token(*stream);
    
    expr_t inner;
    switch (peek.type) {
        case '(': {
            *stream = peek.end;
            if (parse_expr(stream, &inner) != PARSE_OK) {
                return PARSE_BAD;
            }
            if (!expect(stream, ')', "expected `)`", NULL)) {
                free_expr(inner);
                return PARSE_BAD;
            }
            break;
        }
        case '-': {
            *stream = peek.end;
            if (parse_primary(stream, &inner) != PARSE_OK) {
                return PARSE_BAD;
            }
            inner = mk_unop(inner, UNOP_NEGATE);
            break;
        }
        case '!': {
            *stream = peek.end;
            if (parse_primary(stream, &inner) != PARSE_OK) {
                return PARSE_BAD;
            }
            inner = mk_unop(inner, UNOP_NOT);
            break;
        }
        default: {
            if (parse_literal(stream, &inner) != PARSE_OK) {
                return PARSE_BAD;
            };
            break;
        }
    }

    *expr = inner;
    bool cont = true;
    while (cont) {
        peek = read_token(*stream);
        switch (peek.type) {
            case '(': {
                *stream = peek.end;
                            expr_array_t exprs;
                arr_alloc(exprs);

                bool cont2 = true;
                while (cont2) {
                    Token peek = read_token(*stream);

                    switch (peek.type) {
                        case TOKEN_ERROR: {
                            fputs("Syntax error: invalid token.\n", stderr);
                            goto call_expr_loop_cleanup;
                        }
                        case ')': {
                            *stream = peek.end;
                            cont2 = false;
                            break;
                        }
                        case ',': {
                            *stream = peek.end;
                        }
                        default: {
                            expr_t expr;
                            if (parse_expr(stream, &expr) != PARSE_OK) {
                                goto call_expr_loop_cleanup;
                            }
                            arr_append(exprs) = expr;
                            break;
                        }
                    }

                    _: continue;
                    call_expr_loop_cleanup:
                        for (int i = 0; i < arr_get_size(exprs); i++) {
                            free_expr(arr_at(exprs, i));
                        }
                        arr_free(exprs);
                        return PARSE_BAD;
                }
                *expr = mk_call(*expr, exprs);
                break;
            }
            case '[': {
                *stream = peek.end;
                expr_t index;
                if (parse_literal(stream, &index) != PARSE_OK) {
                    free_expr(inner);
                    return PARSE_BAD;
                }
                if (!expect(stream, ']', "expected `]`", NULL)) {
                    free_expr(inner);
                    free_expr(index);
                    return PARSE_BAD;
                }
                *expr = mk_index(*expr, index);
                break;
            }
            default: {
                cont = false;
                break;
            }
        }
    }

    return PARSE_OK;
}

pres_t parse_expr(const char** stream, expr_t* expr) {
    expr_t inner;
    if (parse_primary(stream, &inner) != PARSE_OK) {
        return PARSE_BAD;
    }

    Token peek = read_token(*stream);
    binop_t binop;
    const char* saved_stream = *stream;
    *stream = peek.end;
    switch (peek.type) {
        case TOKEN_DOUBLE_EQUALS: {
            binop = BINOP_EQ;
            break;
        }
        case TOKEN_NOT_EQUALS: {
            binop = BINOP_NE;
            break;
        }
        case TOKEN_GREATER_EQUALS: {
            binop = BINOP_GE;
            break;
        }
        case TOKEN_LESSER_EQUALS: {
            binop = BINOP_LE;
            break;
        }
        case '>': {
            binop = BINOP_GT;
            break;
        }
        case '<': {
            binop = BINOP_LT;
            break;
        }
        case '&': {
            binop = BINOP_AND;
            break;
        }
        case '|': {
            binop = BINOP_OR;
            break;
        }
        case '+': {
            binop = BINOP_ADD;
            break;
        }
        case '-': {
            binop = BINOP_SUB;
            break;
        }
        case '*': {
            binop = BINOP_MUL;
            break;
        }
        case '/': {
            binop = BINOP_DIV;
            break;
        }
        case '%': {
            binop = BINOP_REM;
            break;
        }
        default: {
            *stream = saved_stream;
            *expr = inner;
            return PARSE_OK;
        }
    }

    expr_t rhs;
    if (parse_expr(stream, &rhs) != PARSE_OK) {
        free_expr(inner);
        return PARSE_BAD;
    }

    *expr = mk_binop(inner, rhs, binop);
    return PARSE_OK;
}