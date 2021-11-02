#include <stdint.h>

#include <algorithm>
#include <unordered_map>

#include "ast.hpp"
#include "middle_end.hpp"

using namespace codegen;

template<class> inline constexpr bool always_false_v = false;

IndexName find_or_ident(const std::unordered_map<string, uint64_t>& imap, const string& key) {
    std::unordered_map<string, uint64_t>::const_iterator got = imap.find(key);

    if (got == imap.end()) {
        return got->second;
    } else {
        return key;
    }
}

IndexExpression debify_expression(const StringExpression& e, const std::unordered_map<string, uint64_t>& imap) {
    return std::visit([&](auto& e) -> IndexExpression {
        using T = std::decay_t<decltype(e)>;
        if constexpr (std::is_same_v<T, NullLiteral>) {
            return NullLiteral {};
        } else if constexpr (std::is_same_v<T, BooleanLiteral>) {
            return BooleanLiteral { value: e.value };
        } else if constexpr (std::is_same_v<T, IntegerLiteral>) {
            return IntegerLiteral { value: e.value };
        } else if constexpr (std::is_same_v<T, StringLiteral>) {
            return StringLiteral { value: e.value };
        } else if constexpr (std::is_same_v<T, ListLiteral<string>>) {
            vector<Expression<IndexName>> new_vals = {};

            std::transform(e.value.begin(), e.value.end(), std::back_inserter(new_vals),
                [&](auto& d) -> Expression<IndexName> {
                    return debify_expression(d, imap);
                });

            return ListLiteral<IndexName> {
                value: new_vals,
            };
        } else if constexpr (std::is_same_v<T, Identifier<string>>) {
            std::unordered_map<string, uint64_t> new_imap = imap;
            return Identifier<IndexName> {
                value: find_or_ident(imap, e.value),
            };
        } else if constexpr (std::is_same_v<T, BinaryOperation<string>>) {
            return BinaryOperation<IndexName> {
                left: make_gross<Expression<IndexName>>(debify_expression(e.left.get(), imap)),
                flavor: e.flavor,
                right: make_gross(debify_expression(e.right.get(), imap)),
            };
        } else if constexpr (std::is_same_v<T, UnaryOperation<string>>) {
            return UnaryOperation<IndexName> {
                flavor: e.flavor,
                content: make_gross<Expression<IndexName>>(debify_expression(e.content.get(), imap)),
            };
        } else if constexpr (std::is_same_v<T, Index<string>>) {
            return Index<IndexName> {
                list: make_gross<Expression<IndexName>>(debify_expression(e.list.get(), imap)),
                number: make_gross<Expression<IndexName>>(debify_expression(e.number.get(), imap)),
            };
        } else if constexpr (std::is_same_v<T, Call<string>>) {
            vector<Expression<IndexName>> new_args = {};

            std::transform(e.args.begin(), e.args.end(), std::back_inserter(new_args),
                [&](auto& d) -> Expression<IndexName> {
                    return debify_expression(d, imap);
                });

            return Call<IndexName> {
                function: make_gross<Expression<IndexName>>(debify_expression(e.function.get(), imap)),
                args: new_args,
            };
        } else {
            static_assert(always_false_v<T>, "non-exhaustive visitor!");
        }
    }, e);
}

vector<IndexStatement> debify_statements(
    const vector<StringStatement>& s,
    uint64_t i,
    const std::unordered_map<string, uint64_t>& imap
) {
    std::vector<IndexStatement> new_stats = {};

    std::transform(s.begin(), s.end(), std::back_inserter(new_stats), [&](const StringStatement& d) -> IndexStatement {
            return std::visit([&](auto& d) -> IndexStatement {
                using T = std::decay_t<decltype(d)>;
                if constexpr (std::is_same_v<T, If<string>>) {
                    vector<std::tuple<Expression<IndexName>, vector<Statement<IndexName>>>> new_pairs = {};

                    std::transform(d.if_pairs.begin(), d.if_pairs.end(), std::back_inserter(new_pairs),
                        [&](auto& d) -> std::tuple<Expression<IndexName>, vector<Statement<IndexName>>> {
                            return std::tuple<Expression<IndexName>, vector<Statement<IndexName>>>(
                                debify_expression(std::get<0>(d), imap),
                                debify_statements(std::get<1>(d), i, imap)
                            );
                        });

                    return If<IndexName> {
                        if_pairs: new_pairs,
                        else_body: debify_statements(d.else_body, i, imap),
                    };
                } else if constexpr (std::is_same_v<T, While<string>>) {
                    return While<IndexName> {
                        condition: debify_expression(d.condition, imap),
                        body: debify_statements(d.body, i, imap),
                    };
                } else if constexpr (std::is_same_v<T, Return<string>>) {
                    return Return<IndexName> {
                        content: debify_expression(d.content, imap),
                    };
                } else if constexpr (std::is_same_v<T, Assignment<string>>) {
                    std::unordered_map<string, uint64_t> new_imap = imap;
                    vector<Expression<IndexName>> new_exps = {};

                    std::transform(d.indexes.begin(), d.indexes.end(), std::back_inserter(new_exps),
                        [&](auto& d) -> Expression<IndexName> {
                            return debify_expression(d, imap);
                        });

                    return Assignment<IndexName> {
                        identifier: find_or_ident(imap, d.identifier),
                        indexes: new_exps,
                        content: debify_expression(d.content, imap),
                    };
                } else if constexpr (std::is_same_v<T, VariableDeclaration<string>>) {
                    uint64_t current = i;
                    
                    std::unordered_map<string, uint64_t> new_imap = imap;
                    new_imap[d.identifier] = current;
                    i += 1;

                    return VariableDeclaration<IndexName> {
                        identifier: current,
                        content: debify_expression(d.content, imap),
                    };
                } else if constexpr (std::is_same_v<T, Do<string>>) {
                    return Do<IndexName> {
                        content: debify_expression(d.content, imap),
                    };
                } else { 
                    static_assert(always_false_v<T>, "non-exhaustive visitor!");
                }
            }, d);
        });    

    return new_stats;
}

IndexFunction debify_function(const StringFunction& f) {
    std::unordered_map<string, uint64_t> index_map = {};
    std::vector<IndexName> new_parms = {};

    uint64_t i = 0;
    std::transform(f.parms.begin(), f.parms.end(), std::back_inserter(new_parms), [&](const string& d) -> uint64_t {
            uint64_t current = i;
            index_map.insert({d, current});
            i++;
            return current;
        });

    return IndexFunction {
        identifier: f.identifier,
        // These should always be uints
        parms: new_parms,
        body: debify_statements(f.body, i, index_map),
    };
}

IndexDeclaration debify_declaration(const StringDeclaration& d) {
    return std::visit([](auto& d) -> IndexDeclaration {
        using T = std::decay_t<decltype(d)>;
        if constexpr (std::is_same_v<T, Import>)
            return d;
        else if constexpr (std::is_same_v<T, Global>)
            return d;
        else if constexpr (std::is_same_v<T, StringFunction>)
            return debify_function(d);
        else 
            static_assert(always_false_v<T>, "non-exhaustive visitor!");
    }, d);
}

IndexAST de_bruijnify(const StringAST& sast) {
    std::vector<IndexDeclaration> new_decs = {};

    std::transform(sast.declarations.begin(), sast.declarations.end(), std::back_inserter(new_decs),
        [](const StringDeclaration& d) -> IndexDeclaration { return debify_declaration(d); });

    return AST<IndexName> {
        declarations: new_decs,
    };
}