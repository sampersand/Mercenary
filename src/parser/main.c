#include "ast.h"
#include <stdio.h>
#include "pp.h"

int main(int argc, char **argv) {
    // TODO: actual parser lmao
    stmt_t* while_stmts[2] = {
        mk_do(mk_call(
            mk_ident(mk_string_cstr("print")),
            1,
            box_exprs(1, (expr_t*[1]) { mk_binop(
                mk_index(
                    mk_ident(mk_string_cstr("array")),
                    mk_ident(mk_string_cstr("i"))
                ),
                mk_string_expr(mk_string_cstr("\"a\\n\\x31\\fc\"")),
                BINOP_ADD
            ) })
        )),
        mk_assign_normal(
            mk_string_cstr("i"),
            mk_binop(
                mk_ident(mk_string_cstr("i")),
                mk_number(1),
                BINOP_ADD
            )
        )
    };

    stmt_t* main_stmts[3] = {
        mk_if(
            mk_binop(
                mk_call(
                    mk_ident(mk_string_cstr("length")),
                    1,
                    box_exprs(1, (expr_t*[1]) { mk_ident(mk_string_cstr("argv")) })
                ),
                mk_number(2),
                BINOP_EQ
            ),
            mk_block(
                1,
                box_stmts(1, (stmt_t*[1]) { mk_assign_normal(
                    mk_string_cstr("result"),
                    mk_call(
                        mk_ident(mk_string_cstr("fizzbuzz")),
                        1,
                        box_exprs(1, (expr_t*[1]) { mk_call(
                            mk_ident(mk_string_cstr("atoi")),
                            1,
                            box_exprs(1, (expr_t*[1]) { mk_index(
                                mk_ident(mk_string_cstr("argv")),
                                mk_number(1)
                            ) })
                        ) })
                    )
                ) })
            ),
            0,
            NULL,
            NULL,
            mk_block(
                1,
                box_stmts(1, (stmt_t*[1]) { mk_assign_normal(
                    mk_string_cstr("result"),
                    mk_call(
                        mk_ident(mk_string_cstr("fizzbuzz")),
                        1,
                        box_exprs(1, (expr_t*[1]) { mk_number(100) })
                    )
                ) })
            )
        ),
        mk_assign_normal(
            mk_string_cstr("i"),
            mk_number(1)
        ),
        mk_while(
            mk_binop(
                mk_ident(mk_string_cstr("i")),
                mk_call(
                    mk_ident(mk_string_cstr("length")),
                    1,
                    box_exprs(1, (expr_t*[1]) { mk_ident(mk_string_cstr("array")) })
                ),
                BINOP_NE
            ),
            mk_block(
                2,
                box_stmts(2, while_stmts)
            )
        )
    };
    block_t* main_block = mk_block(
                3,
                box_stmts(3, main_stmts)
            );
    string_t argv_ident = mk_string_cstr("argv");
    program_t* program = mk_program(
        1,
        box_decls(1, (decl_t*[1]) { mk_fn_decl(
            mk_string_cstr("main"),
            1,
            box_strings(1, &argv_ident),
            main_block
        ) })
    );

    pp_program(program);

    free_program(program);
    return 0;
}