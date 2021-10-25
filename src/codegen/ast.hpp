#ifndef AST_CODEGEN
#define AST_CODEGEN

#include <stdint.h>

extern "C"
{
    #include "../parser/ast.h"
}

#include <memory>
#include <string>
#include <tuple>
#include <vector>
#include <variant>

using string = std::string;

template<typename T>
using vector = std::vector<T>;

namespace codegen {
    // Forward declaration, so the recursive variant works

    struct NullLiteral;
    struct BooleanLiteral;
    struct IntegerLiteral;
    struct StringLiteral;
    
    template<typename T>
    struct ListLiteral;

    template<typename T>
    struct Identifier;

    template<typename T>
    struct BinaryOperation;

    template<typename T>
    struct UnaryOperation;

    template<typename T>
    struct Index;

    template<typename T>
    struct Call;

    template<typename T>
    using Expression = std::variant<
        NullLiteral, BooleanLiteral, IntegerLiteral,
        StringLiteral, ListLiteral<T>, Identifier<T>,
        BinaryOperation<T>, UnaryOperation<T>, Index<T>,
        Call<T>
    >;

    struct NullLiteral {};

    struct BooleanLiteral {
        bool value;
    };

    struct IntegerLiteral {
        int64_t value;
    };

    struct StringLiteral {
        string value;
    };

    template<typename T>
    struct ListLiteral {
        vector<Expression<T>> value;
    };

    template<typename T>
    struct Identifier {
        T value;
    };

    enum BinaryFlavor {
        Equal,
        NotEqual,
        GreaterThan,
        GreaterThanOrEqual,
        LessThan,
        LessThanOrEqual,
        And,
        Or,
        Addition,
        Subtraction,
        Multiplication,
        Division,
        ModulousOrRemainder,
    };

    template<typename T>
    struct BinaryOperation {
        std::shared_ptr<Expression<T>> left;
        BinaryFlavor flavor;
        std::shared_ptr<Expression<T>> right;
    };

    enum UnaryFlavor {
        Negate,
        Not,
    };

    template<typename T>
    struct UnaryOperation {
        UnaryFlavor flavor;
        std::shared_ptr<Expression<T>> contents;
    };

    template<typename T>
    struct Index {
        std::shared_ptr<Expression<T>> list;
        std::shared_ptr<Expression<T>> number;
    };

    template<typename T>
    struct Call {
        std::shared_ptr<Expression<T>> function;
        vector<Expression<T>> args;
    };

    // More forward declaration
    template<typename T>
    struct Do;

    template<typename T>
    struct VariableDeclaration;

    template<typename T>
    struct Assignment;

    template<typename T>
    struct Return;

    template<typename T>
    struct While;

    template<typename T>
    struct If;

    template<typename T>
    using Statement = std::variant<
        If<T>, While<T>, Return<T>,
        Assignment<T>, VariableDeclaration<T>, Do<T>
    >;

    template<typename T>
    struct Do {
        Expression<T> content;
    };

    template<typename T>
    struct VariableDeclaration {
        T identifier;
        Expression<T> content;
    };

    template<typename T>
    struct Assignment {
        T identifier;
        vector<Expression<T>> indexes; 
        Expression<T> content;
    };

    template<typename T>
    struct Return {
        Expression<T> content;
    };

    template<typename T>
    struct While {
        Expression<T> condition;
        vector<Statement<T>> body;
    };

    template<typename T>
    struct If {
        vector<std::tuple<Expression<T>, vector<Statement<T>>>> if_pairs;
        vector<Statement<T>> else_body;
    };

    struct Import {
        string path;
    };

    struct Global {
        string identifier;
    };

    template<typename T>
    struct Function {
        string identifier;
        vector<T> parms;
        vector<Statement<T>> body;
    };

    template<typename T>
    using Declaration = std::variant<Import, Global, Function<T>>;

    // AST is a template so that it can go from string to indexes without being duplicated
    template<typename T>
    struct AST {
        vector<Declaration<T>> declarations;
    };

    using StringAST = AST<string>;
    using StringFunction = Function<string>;
    using StringStatement = Statement<string>;
    using StringExpression = Expression<string>;

    using IndexAST = AST<uint64_t>;
    using IndexFunction = Function<uint64_t>;
    using IndexStatement = Statement<uint64_t>;
    using IndexExpression = Expression<uint64_t>;

    StringAST to_cpp_ast(program_t);
}

#endif