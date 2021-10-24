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
struct Identifier;
struct ListLiteral;
struct IntegerLiteral;
struct BooleanLiteral;
struct NullLiteral;
struct BinaryOperation;
struct UnaryOperation;

using Expression = boost::variant<boost::recursive_wrapper<UnaryOperation>, boost::recursive_wrapper<BinaryOperation>,
    NullLiteral, BooleanLiteral, IntegerLiteral, ListLiteral, Identifier
>;

struct Identifier {
    string value;
};

struct ListLiteral {
    vector<Expression> value;
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

struct BinaryOperation {
    BinaryFlavor binary_flavor;
    unique_ptr<Expression> left;
    unique_ptr<Expression> right;
};

enum UnaryFlavor {
    Negate,
    Not,
};

struct UnaryOperation {
    UnaryFlavor unary_flavor;
    unique_ptr<Expression> contents;
};

// More forward declaration
struct Do;
struct VariableDeclaration;
struct Assignment;
struct Return;
struct While;
struct If;

using Statement = boost::variant<boost::recursive_wrapper<If>, boost::recursive_wrapper<While>, 
    Return, Assignment, VariableDeclaration, Do
>;

struct Do {
    Expression content;
};

struct VariableDeclaration {
    string identifier;
    Expression content;
};

struct Assignment {
    string identifier;
    Expression content;
};

struct Return {
    Expression content;
};

struct While {
    Expression condition;
    vector<Statement> body;
};

struct If {
    Expression condition;
    vector<Statement> on_true;
    vector<Statement> on_false;
};

struct Import {
    string path;
};

struct Global {
    string identifier;
};

struct Function {
    string identifier;
    vector<string> parms;
    vector<Statement> body;
};

using Declaration = std::variant<Import, Global, Function>;

struct AST {
    std::vector<Declaration> declarations;
};