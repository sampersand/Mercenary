#include "parser.h"
#include <stdlib.h>
#include <stdio.h>

#include "dyn_array.h"

// TODO proper error handling lmao

pres_t expect(const char** stream, uint64_t type, const char* error, Token* tok, bool peek) {
    Token t = read_token(*stream);
    if (!peek) {
        *stream = t.end;
    }

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
            if (!expect(stream, TOKEN_STRING, "expected string", &string, false) ||
                !expect(stream, ';', "expected semicolon", NULL, false)) {
                return PARSE_BAD;
            }
            *decl = mk_import(mk_string_2ptrs(string.start, string.end));
            break;
        }
        case TOKEN_GLOBAL: {
            Token ident;
            if (!expect(stream, TOKEN_IDENTIFIER, "expected identifier", &ident, false) ||
                !expect(stream, ';', "expected semicolon", NULL, false)) {
                return PARSE_BAD;
            }
            *decl = mk_global(mk_string_2ptrs(ident.start, ident.end));
            break;
        }
        case TOKEN_FUNCTION: {
            Token name;
            if (!expect(stream, TOKEN_IDENTIFIER, "expected identifier", &name, false) ||
                !expect(stream, '(', "expected opening paren", NULL, false)) {
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
                        if (!expect(stream, TOKEN_IDENTIFIER, "expected ident", &ident, false)) {
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

    if (!expect(stream, '{', "expected `{`", NULL, false)) {
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

    if (!expect(stream, '}', "expected `}`", NULL, false)) {
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
            if (!expect(stream, ';', "expected `;`", NULL, false)) {
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
            if (!expect(stream, ';', "expected `;`", NULL, false)) {
                free_expr(expr);
                return PARSE_BAD;
            }
            *stmt = mk_do(expr);
            break;
        }
        case TOKEN_LET: {
            Token ident;
            expr_t value;
            if (!expect(stream, TOKEN_IDENTIFIER, "expected ident", &ident, false) ||
                !expect(stream, '=', "expected `=`", NULL, false) ||
                parse_expr(stream, &value) != PARSE_OK) {
                    return PARSE_BAD;
            }
            if (!expect(stream, ';', "expected `;`", NULL, false)) {
                free_expr(value);
                return PARSE_BAD;
            }
            *stmt = mk_declare_var(mk_string_2ptrs(ident.start, ident.end), value);
            break;
        }
        case TOKEN_WHILE: {
            expr_t cond;
            block_t block;
            if (!expect(stream, '(', "expected `(`", NULL, false) ||
                parse_expr(stream, &cond) != PARSE_OK) {
                return PARSE_BAD;
            }
            if (!expect(stream, ')', "expected `)`", NULL, false) ||
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

            if (!expect(stream, '(', "expected `(`", NULL, false) ||
                parse_expr(stream, &main_cond) != PARSE_OK) {
                arr_free(elif_conds);
                arr_free(elif_blocks);
                return PARSE_BAD;
            }
            if (!expect(stream, ')', "expected `)`", NULL, false) ||
                parse_block(stream, &main_block) != PARSE_OK) {
                free_expr(main_cond);
                arr_free(elif_conds);
                arr_free(elif_blocks);
                return PARSE_BAD;
            }

            bool cont = true;
            while (cont) {
                Token peek = read_token(*stream);
                switch (peek.type) {
                    case TOKEN_ERROR: {
                        fputs("Syntax error: invalid token.\n", stderr);
                        free_expr(main_cond);
                        free_block(main_block);
                        return PARSE_BAD;
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
                            // EOF
                            case 0: {
                                fputs("Syntax error: expected `if` or `{`.\n", stderr);
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
                            case TOKEN_IF: {
                                *stream = peek1.end;
                                expr_t cond;
                                block_t block;
                                if (!expect(stream, '(', "expected `(`", NULL, false) ||
                                    parse_expr(stream, &cond) != PARSE_OK) {
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
                                if (!expect(stream, ')', "expected `)`", NULL, false) ||
                                    parse_block(stream, &block) != PARSE_OK) {
                                    free_expr(main_cond);
                                    free_block(main_block);
                                    for (int i = 0; i < arr_get_size(elif_conds); i++) {
                                        free_expr(arr_at(elif_conds, i));
                                        free_block(arr_at(elif_blocks, i));
                                    }
                                    free_expr(cond);
                                    arr_free(elif_conds);
                                    arr_free(elif_blocks);
                                    return PARSE_BAD;
                                }
                                arr_append(elif_conds) = cond;
                                // NOLINTNEXTLINE(bugprone-sizeof-expression)
                                arr_append(elif_blocks) = block;
                                break;
                            }
                            default: {
                                if (parse_block(stream, &else_block) != PARSE_OK) {
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
            }

            *stmt = mk_if(main_cond, main_block, elif_conds, elif_blocks, else_block);
            return PARSE_OK;
        }
        default: {
            fputs("Syntax error: expected `if`, `while`, `return`, `let`, `set`, or `do`.\n", stderr);
            return PARSE_BAD;
        }
    }


    return PARSE_OK;
}

pres_t parse_expr(const char** stream, expr_t* expr) {
    *expr = mk_null();
    return PARSE_OK;
}