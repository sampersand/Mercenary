#include <stdio.h>

extern "C" {
    #include "../parser/ast.h"
    #include "../parser/dyn_array.h"
}

#include "ast.hpp"

#include <set>

using namespace codegen;

std::monostate panic() {
    volatile int *p = NULL;
    *p = 1;
    return std::monostate {};
}

string parseHex(const string& s) {
    std::set<char> valid_chars = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'a', 'b', 'c', 'd', 'e', 'f',
    };

    if (s.length() < 2 && valid_chars.contains(std::tolower(s[0])) && valid_chars.contains(std::tolower(s[1]))) {
        panic();
    }
    
    string hex = s.substr(0, 2);
    for(char& c : hex)
    {
        c = tolower(c);
    }

    int len = hex.length();
    string new_string = "";

    for(int i = 0; i < len; i += 2)
    {
        string byte = hex.substr(i, 2);
        char chr = (char) (int)strtol(byte.c_str(), NULL, 16);
        new_string.push_back(chr);
    }

    return new_string;
}

string unescape(const string& s) {
    string output = "";

    for (auto it = s.cbegin(); it != s.cend(); it++) {
        const int64_t next_i = -std::distance(it, s.cbegin()) + 1;

        if (*it == '\\') {
            if (next_i >= s.length()) {
                panic();
            } else {
                switch (s[next_i])
                {
                case 'n':
                    output.push_back('\n');
                    break;

                case 't':
                    output.push_back('\t');
                    break;

                case 'f':
                    output.push_back('\f');
                    break;

                case 'r':
                    output.push_back('\r');
                    break;

                case '\"':
                    output.push_back('\"');
                    break;

                case '\'':
                    output.push_back('\'');
                    break;

                case '\\':
                    output.push_back('\\');
                    break;

                case 'x':
                    output.append(parseHex(s.substr(next_i + 1)));
                    it += 3;
                    break;
                
                default:
                    panic();
                    break;
                };

                it++;
            };
        } else {
            output.push_back(*it);
        };
    };

    return output;
}

string to_cpp_str(string_t str, bool is_lit = false) {
    char new_str[str.len];
    sprintf(new_str, "%2$.*1$s", str.len, str.s);
    string new_cpp = new_str;

    if (is_lit) {
        return new_cpp.substr(1, new_cpp.size() - 2);
    } else {
        return new_cpp;
    }
}

BinaryFlavor to_cpp_binary(binop_t binary_op_c) {
    switch (binary_op_c) {
        case binop_t::BINOP_EQ:
            return BinaryFlavor::Equal;
            break;
        
        case binop_t::BINOP_NE:
            return BinaryFlavor::NotEqual;
            break;

        case binop_t::BINOP_GT:
            return BinaryFlavor::GreaterThan;
            break;

        case binop_t::BINOP_GE:
            return BinaryFlavor::GreaterThanOrEqual;
            break;

        case binop_t::BINOP_LT:
            return BinaryFlavor::LessThan;
            break;

        case binop_t::BINOP_LE:
            return BinaryFlavor::LessThanOrEqual;
            break;

        case binop_t::BINOP_AND:
            return BinaryFlavor::And;
            break;

        case binop_t::BINOP_OR:
            return BinaryFlavor::Or;
            break;

        case binop_t::BINOP_ADD:
            return BinaryFlavor::Addition;
            break;

        case binop_t::BINOP_SUB:
            return BinaryFlavor::Subtraction;
            break;

        case binop_t::BINOP_MUL:
            return BinaryFlavor::Multiplication;
            break;

        case binop_t::BINOP_DIV:
            return BinaryFlavor::Division;
            break;

        case binop_t::BINOP_REM:
            return BinaryFlavor::ModulousOrRemainder;
            break;

        default:
            panic();
            return BinaryFlavor::ModulousOrRemainder; // Unreachable, makes the compiler happy 
            break;
    }
}


UnaryFlavor to_cpp_unary(unop_t unary_op_c) {
    switch (unary_op_c) {
        case unop_t::UNOP_NEGATE:
            return UnaryFlavor::Negate;
            break;
        case unop_t::UNOP_NOT:
            return UnaryFlavor::Not;
            break;
        default:
            panic();
            return UnaryFlavor::Not; // Unreachable, makes the compiler happy 
            break;
    }
}

StringExpression to_cpp_expression(expr_t* expr_c) {
    switch (expr_c->kind) {
    case expr::EXPR_BINARY:
        return BinaryOperation<string> {
            left: make_gross<StringExpression>(to_cpp_expression(expr_c->value.binary.lhs)),
            flavor: to_cpp_binary(expr_c->value.binary.op),
            right: make_gross<StringExpression>(to_cpp_expression(expr_c->value.binary.rhs)),
        };
        break;

    case expr::EXPR_UNARY:
        return UnaryOperation<string> {
            flavor: to_cpp_unary(expr_c->value.unary.op),
            content: make_gross<StringExpression>(to_cpp_expression(expr_c->value.unary.subexpr)),
        };
        break;

    case expr::EXPR_CALL: {
        vector<StringExpression> args = {};

        for (size_t i = 0; i < arr_get_size(expr_c->value.call.args); i++) {
            args.push_back(to_cpp_expression(&arr_at(expr_c->value.call.args, i)));
        }

        return Call<string> {
            function: make_gross<StringExpression>(to_cpp_expression(expr_c->value.call.func)),
            args: args,
        };
        break;
    }

    case expr::EXPR_INDEX:
        return Index<string> {
            list: make_gross<StringExpression>(to_cpp_expression(expr_c->value.index.array)),
            number: make_gross<StringExpression>(to_cpp_expression(expr_c->value.index.index)),
        };
        break;

    case expr::EXPR_BOOL:
        return BooleanLiteral { value: expr_c->value.bool_expr};
        break;

    case expr::EXPR_NUMBER:
        // This cast may not make sense, I'm not sure what the parse is doing?
        return IntegerLiteral { value: static_cast<int64_t>(expr_c->value.number) };
        break;

    case expr::EXPR_STRING:
        return StringLiteral { value: unescape(to_cpp_str(expr_c->value.string, true)) };
        break;

    case expr::EXPR_IDENT:
        return Identifier<string> { value: to_cpp_str(expr_c->value.string) };
        break;

    case expr::EXPR_ARRAY: {
        vector<StringExpression> args = {};

        for (size_t i = 0; i < arr_get_size(expr_c->value.array); i++) {
            args.push_back(to_cpp_expression(&arr_at(expr_c->value.array, i)));
        }

        return ListLiteral<string> { value: args };
        break;
    }

    case expr::EXPR_NULL:
        return NullLiteral {};
        break;
    
    default:
        panic();
        return NullLiteral {}; // Unreachable, makes the compiler happy
        break;
    }
}

// Forward declaration for blocks
vector<StringStatement> to_cpp_body(block_t*);

StringStatement to_cpp_statement(stmt_t* stmt_c) {
    switch (stmt_c->kind) {
    case stmt_t::STMT_IF: {
        if_stmt_t if_c = stmt_c->value.if_stmt;
        vector<std::tuple<Expression<string>, vector<Statement<string>>>> if_pairs = {};

        if_pairs.push_back(std::tuple<Expression<string>, vector<Statement<string>>>{
            to_cpp_expression(if_c.main_cond),
            to_cpp_body(&if_c.main_block)
        });

        for (size_t i = 0; i < arr_get_size(if_c.elif_conds); i++) {
            if_pairs.push_back(std::tuple<Expression<string>, vector<Statement<string>>>{
                to_cpp_expression(&arr_at(if_c.elif_conds, i)),
                to_cpp_body(&arr_at(if_c.elif_blocks, i))
            });
        }

        std::optional<vector<StringStatement>> body = stmt_c->value.if_stmt.else_block == NULL ? 
            std::nullopt
            : std::optional<vector<StringStatement>>(to_cpp_body(&stmt_c->value.if_stmt.else_block));

        if (stmt_c->value.if_stmt.else_block != NULL) {
        }

        return If<string> {
            if_pairs: if_pairs,
            else_body: body,
        };
        return Do<string> {
            content: NullLiteral {},
        };
        break;
    }

    case stmt_t::STMT_WHILE:
        return While<string> {
            condition: to_cpp_expression(stmt_c->value.while_stmt.cond),
            body: to_cpp_body(&stmt_c->value.while_stmt.block),
        };
        break;

    case stmt_t::STMT_RETURN:
        return Return<string> {
            content: to_cpp_expression(stmt_c->value.expr),
        };
        break;

    case stmt_t::STMT_DECLARE_VAR:
        return VariableDeclaration<string> {
            identifier: to_cpp_str(stmt_c->value.declare_var.ident),
            content: to_cpp_expression(stmt_c->value.declare_var.value),
        };
        break;

    case stmt_t::STMT_ASSIGN_VAR: {
        vector<StringExpression> indexes = {};

        for (size_t i = 0; i < arr_get_size(stmt_c->value.assign_var.indices); i++) {
            indexes.push_back(to_cpp_expression(&arr_at(stmt_c->value.assign_var.indices, i)));
        }

        return Assignment<string> {
            identifier: to_cpp_str(stmt_c->value.assign_var.ident),
            indexes: indexes,
            content: to_cpp_expression(stmt_c->value.assign_var.value),
        };
        break;
    }

    case stmt_t::STMT_DO:
        return Do<string> {
            content: to_cpp_expression(stmt_c->value.expr),
        };
        break;

    default:
        panic();
        return Do<string> { content: to_cpp_expression(stmt_c->value.expr) }; // Unreachable, makes the compiler happy 
        break;
    }
}

vector<StringStatement> to_cpp_body(block_t* body_c) {
    vector<StringStatement> statements = {};

    for (size_t i = 0; i < arr_get_size(*body_c); i++) {
        statements.push_back(to_cpp_statement(&arr_at(*body_c, i)));
    }

    return statements;
}

StringFunction to_cpp_function(fn_decl_t* function_c) {
    vector<string> parms = {};

    for (size_t i = 0; i < arr_get_size(function_c->args); i++) {
        parms.push_back(to_cpp_str(arr_at(function_c->args, i)));
    }

    return StringFunction {
        identifier: to_cpp_str(function_c->name),
        parms: parms,
        body: to_cpp_body(&function_c->block),
    };
}

namespace codegen {
    StringAST to_cpp_ast(program_t* program_c) {
        vector<StringDeclaration> declarations = {};

        for (size_t i = 0; i < arr_get_size(*program_c); i++) {
            decl_t declaration_c = arr_at(*program_c, i);

            switch (declaration_c.kind) {
            case decl_t::DECL_FUNCTION:
                declarations.push_back(to_cpp_function(&declaration_c.value.fn));
                break;

            case decl_t::DECL_GLOBAL:
                declarations.push_back(Global { identifier: to_cpp_str(declaration_c.value.string) });
                break;

            case decl_t::DECL_IMPORT:
                declarations.push_back(Import { path: to_cpp_str(declaration_c.value.string) });
                break;
            
            default: 
                panic();
                break;
            }
        }

        return StringAST { declarations: declarations };
    }
}