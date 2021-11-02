#include <algorithm>
#include <tuple>

#include "instructions.hpp"

using namespace codegen;

template<class> inline constexpr bool always_false_v = false;

std::monostate panic2() {
    *(int*)0 = 0;
    return std::monostate {};
}

string stringify_binop(BinaryFlavor b) {
    switch (b) {
    case BinaryFlavor::Equal:
        return "~==";
        break;

    case BinaryFlavor::NotEqual:
        return "~!=";
        break;

    case BinaryFlavor::GreaterThan:
        return "~>";
        break;

    case BinaryFlavor::GreaterThanOrEqual:
        return "~>=";
        break;

    case BinaryFlavor::LessThan:
        return "~<";
        break;

    case BinaryFlavor::LessThanOrEqual:
        return "~<=";
        break;

    case BinaryFlavor::And:
        return "~&&";
        break;

    case BinaryFlavor::Or:
        return "~||";
        break;

    case BinaryFlavor::Addition:
        return "~+";
        break;

    case BinaryFlavor::Subtraction:
        return "~-";
        break;

    case BinaryFlavor::Multiplication:
        return "~*";
        break;

    case BinaryFlavor::Division:
        return "~/";
        break;

    case BinaryFlavor::ModulousOrRemainder:
        return "~%";
        break;
    
    default:
        panic2();
        return "~!!!";
        break;
    }
}

string stringify_unop(UnaryFlavor b) {
    switch (b) {
    case UnaryFlavor::Negate:
        return "#-";
        break;

    case UnaryFlavor::Not:
        return "#!";
        break;
    
    default:
        panic2();
        return "#!!!";
        break;
    }
}

IIdentifier make_iident(const string& s) {
    return IIdentifier { value: new string(s) };
}

Arity make_arity(uint64_t a) {
    return Arity { value: a };
}

LocalIndex make_index(uint64_t i) {
    return LocalIndex { value: i };
}

Instructions insify_expression(const IndexExpression& e) {
    return std::visit([](auto& e) -> Instructions {
        using T = std::decay_t<decltype(e)>;
        if constexpr (std::is_same_v<T, NullLiteral>) {
            return {NullConst {}};
        } else if constexpr (std::is_same_v<T, BooleanLiteral>) {
            return {BooleanConst { value: e.value }};
        } else if constexpr (std::is_same_v<T, IntegerLiteral>) {
            return {IntegerConst { value: e.value }};
        } else if constexpr (std::is_same_v<T, StringLiteral>) {
            string* copyVal = new string(e.value);
            return {StringConst { value: copyVal }};
        } else if constexpr (std::is_same_v<T, ListLiteral<IndexName>>) {
            Instructions ins  = {};
            
            for (IndexExpression expr : e.value) {
                Instructions expr_ins = insify_expression(expr);
                ins.insert(expr_ins.end(), expr_ins.begin(), expr_ins.end());
            }

            ins.push_back(ListConst { value: e.value.size() });
            return ins;
        } else if constexpr (std::is_same_v<T, Identifier<IndexName>>) {
            return std::visit([](auto& id) -> Instructions {
                using T = std::decay_t<decltype(id)>;
                if constexpr (std::is_same_v<T, uint64_t>) {
                    return {GetLocal { index: make_index(id) }};
                } else if constexpr (std::is_same_v<T, string>) {
                    return {StringConst { value: new string(id) }, GetFree {}};
                } else {
                    static_assert(always_false_v<T>, "non-exhaustive visitor!");
                }
            }, e.value);
        } else if constexpr (std::is_same_v<T, BinaryOperation<IndexName>>) {
            Instructions left = insify_expression(e.left.get());
            Instructions right = insify_expression(e.right.get());

            left.insert(right.end(), right.begin(), right.end());
            left.push_back(CallKnown { arg_count: make_arity(2), ident: make_iident(stringify_binop(e.flavor)) });

            return left;
        } else if constexpr (std::is_same_v<T, UnaryOperation<IndexName>>) {
            Instructions ins = insify_expression(e.content.get());

            ins.push_back(CallKnown { arg_count: make_arity(1), ident: make_iident(stringify_unop(e.flavor)) });
            return ins;
        } else if constexpr (std::is_same_v<T, Index<IndexName>>) {
            Instructions ins = insify_expression(e.list.get());
            Instructions number = insify_expression(e.number.get());
            
            ins.insert(number.end(), number.begin(), number.end());
            ins.push_back(CallKnown { arg_count: make_arity(2), ident: make_iident("~[]") });
            return ins;
        } else if constexpr (std::is_same_v<T, Call<IndexName>>) {
            Instructions ins  = {};
            
            for (IndexExpression expr : e.args) {
                Instructions arg_ins = insify_expression(expr);
                ins.insert(arg_ins.end(), arg_ins.begin(), arg_ins.end());
            }

            Instructions name_ins = insify_expression(e.function.get());
            ins.insert(name_ins.end(), name_ins.begin(), name_ins.end());
            ins.push_back(CallUnknown { arg_count: make_arity(e.args.size()) });
            return ins;
        } else {
            static_assert(always_false_v<T>, "non-exhaustive visitor!");
        }
    }, e);
}

// Forward declared for while, if
Instructions insify_statements(const vector<IndexStatement>& s);

Instructions insify_if(
    const vector<std::tuple<Expression<IndexName>, vector<Statement<IndexName>>>>& pairs,
    const vector<Statement<IndexName>>& else_body)
{
    if (pairs.size() == 0) {
        return insify_statements(else_body);
    } else {
        Instructions ins = {};
        Instructions cond = insify_expression(std::get<0>(pairs[0]));
        ins.insert(cond.end(), cond.begin(), cond.end());

        ins.push_back(StartBlock {});
        Instructions first_body = insify_statements(std::get<1>(pairs[0]));
        ins.insert(first_body.end(), first_body.begin(), first_body.end());
        ins.push_back(EndBlock {});

        ins.push_back(StartBlock {});
        Instructions rest_body = insify_if(
            std::vector<std::tuple<Expression<IndexName>, vector<Statement<IndexName>>>>(
                pairs.begin() + 1,
                pairs.end()
            ),
            else_body
        );
        ins.insert(rest_body.end(), rest_body.begin(), rest_body.end());
        ins.push_back(EndBlock {});
        
        ins.push_back(IIf {});

        return ins;
    }
}

Instructions insify_statement(const IndexStatement& s) {
    return std::visit([](auto& s) -> Instructions {
        using T = std::decay_t<decltype(s)>;
        if constexpr (std::is_same_v<T, If<IndexName>>) {
            return insify_if(s.if_pairs, s.else_body);
        } else if constexpr (std::is_same_v<T, While<IndexName>>) {
            Instructions ins = {};
            ins.push_back(StartBlock {});

            Instructions cond_ins = insify_expression(s.condition);
            ins.insert(cond_ins.end(), cond_ins.begin(), cond_ins.end());

            ins.push_back(BreakIf {});

            Instructions body_ins = insify_statements(s.body);
            ins.insert(body_ins.end(), body_ins.begin(), body_ins.end());

            ins.push_back(EndBlock {});
            ins.push_back(Loop {});
            return {};
        } else if constexpr (std::is_same_v<T, Return<IndexName>>) {
            Instructions ins = {};

            Instructions expr_ins = insify_expression(s.content);
            ins.insert(expr_ins.end(), expr_ins.begin(), expr_ins.end());

            ins.push_back(IReturn {});
            return ins;
        } else if constexpr (std::is_same_v<T, Assignment<IndexName>>) {;
            Instructions ins = std::visit([](auto& id) -> Instructions {
                using T = std::decay_t<decltype(id)>;
                if constexpr (std::is_same_v<T, uint64_t>) {
                    return {GetLocal { index: make_index(id) }};
                } else if constexpr (std::is_same_v<T, string>) {
                    return {StringConst { value: new string(id) }, GetFree {}};
                } else {
                    static_assert(always_false_v<T>, "non-exhaustive visitor!");
                }
            }, s.identifier);

            for (auto iter = s.indexes.begin(); iter != std::prev(s.indexes.end()); ++iter) {
                Instructions ie_ins = insify_expression(*iter);
                ins.insert(ie_ins.end(), ie_ins.begin(), ie_ins.end());
                ins.push_back(CallKnown { arg_count: make_arity(2), ident: make_iident("~[]") });
            }

            Instructions last_ins = insify_expression(s.indexes[s.indexes.size() - 1]);
            ins.insert(last_ins.end(), last_ins.begin(), last_ins.end());

            Instructions con_ins = insify_expression(s.content);
            ins.insert(con_ins.end(), con_ins.begin(), con_ins.end());

            ins.push_back(CallKnown { arg_count: make_arity(3), ident: make_iident("==[]") });      

            return ins;
        } else if constexpr (std::is_same_v<T, VariableDeclaration<IndexName>>) {
            Instructions ins = {};

            Instructions expr_ins = insify_expression(s.content);
            ins.insert(expr_ins.end(), expr_ins.begin(), expr_ins.end());

            ins.push_back(SetLocal { index: make_index(std::get<uint64_t>(s.identifier)) });

            return ins;
        } else if constexpr (std::is_same_v<T, Do<IndexName>>) {
            Instructions ins = {};

            Instructions expr_ins = insify_expression(s.content);
            ins.insert(expr_ins.end(), expr_ins.begin(), expr_ins.end());

            ins.push_back(Drop {});
            return ins;
        } else { 
            static_assert(always_false_v<T>, "non-exhaustive visitor!");
        }
    }, s);
}

Instructions insify_statements(const vector<IndexStatement>& s) {
    Instructions ins = {};

    for (IndexStatement stmt : s) {
        Instructions stmt_ins = insify_statement(stmt);
        ins.insert(stmt_ins.end(), stmt_ins.begin(), stmt_ins.end());
    }

    return ins;
}

Instructions insify_declaration(const IndexDeclaration& d) {
    return std::visit([](auto& d) -> Instructions {
        using T = std::decay_t<decltype(d)>;
        if constexpr (std::is_same_v<T, Import>) {
            return {StringConst { value: new string(d.path) }, IImport {}};
        } else if constexpr (std::is_same_v<T, Global>) {
            return {StringConst { value: new string(d.identifier) }, IGlobal {}};
        } else if constexpr (std::is_same_v<T, IndexFunction>) {
            Instructions ins = {StartBlock {}};

            Instructions body_ins = insify_statements(d.body);
            ins.insert(body_ins.end(), body_ins.begin(), body_ins.end());

            ins.push_back(EndBlock {});
            ins.push_back(IFunc { parm_count: make_arity(d.parms.size()), ident: make_iident(d.identifier) });
            return ins;
        } else {
            static_assert(always_false_v<T>, "non-exhaustive visitor!");
        }
    }, d);
}

Instructions instructionify(const IndexAST& iast) {
    Instructions ins = {};

    for (IndexDeclaration dec : iast.declarations) {
        Instructions dec_ins = insify_declaration(dec);
        ins.insert(dec_ins.end(), dec_ins.begin(), dec_ins.end());
    }

    return ins;
}

string intruction_to_string(const Instruction& ins) {
    
}
string intructions_to_string(const Instructions& ins) {
    
}