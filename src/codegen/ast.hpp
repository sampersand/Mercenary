#ifndef AST_CODEGEN
#define AST_CODEGEN

#include <stdint.h>

#include <memory>
#include <string>
#include <vector>
#include <variant>

#include <boost/variant.hpp>

using string = std::string;

template<typename T>
using vector = std::vector<T>;

template<typename T>
using unique_ptr = std::unique_ptr<T>;

// Forward declaration, so the recursive variant works
template<typename T>
struct Identifier;

template<typename T>
struct ListLiteral;
struct IntegerLiteral;
struct BooleanLiteral;
struct NullLiteral;

template<typename T>
struct BinaryOperation;

template<typename T>
struct UnaryOperation;

template<typename T>
using Expression = std::variant<boost::recursive_wrapper<UnaryOperation<T>>, boost::recursive_wrapper<BinaryOperation<T>>,
    NullLiteral, BooleanLiteral, IntegerLiteral, ListLiteral<T>, Identifier<T>
>;

template<typename T>
struct Identifier {
    T value;
};

template<typename T>
struct ListLiteral {
    vector<Expression<T>> value;
};

struct IntegerLiteral {
    uint64_t value;
};

struct BooleanLiteral {
    bool value;
};

struct NullLiteral {};

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
    BinaryFlavor binary_flavor;
    unique_ptr<Expression<T>> left;
    unique_ptr<Expression<T>> right;
};

enum UnaryFlavor {
    Negate,
    Not,
};

template<typename T>
struct UnaryOperation {
    UnaryFlavor unary_flavor;
    unique_ptr<Expression<T>> contents;
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
using Statement = std::variant<boost::recursive_wrapper<If<T>>, boost::recursive_wrapper<While<T>>, 
    Return<T>, Assignment<T>, VariableDeclaration<T>, Do<T>
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
    Expression<T> condition;
    vector<Statement<T>> on_true;
    vector<Statement<T>> on_false;
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
    std::vector<Declaration<T>> declarations;
};

using StringAST = unique_ptr<AST<string>>;
using IndexAst = unique_ptr<AST<uint64_t>>;

string unescape(const string&);

#endif