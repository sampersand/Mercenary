#include "parser.h"
#include <stdlib.h>
#include <stdio.h>

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
                arr_free(*program);
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
            arr_alloc(block);
            // TODO: blocks
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