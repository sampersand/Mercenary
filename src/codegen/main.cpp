#include <iostream>

#include "ast.hpp"
#include "middle_end.hpp"
#include "instructions.hpp"

using namespace codegen;

int main()
{
    Instructions ins = instructionify(AST<IndexName> { declarations: {Import { path: "ok"}}});
    std::cout << intructions_to_string(ins) << std::endl;
}