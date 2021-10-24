#include "ast.hpp"

#include <iostream>

int main() 
{
    string foo = "Hello\\n World!";
    const string& foo2 = foo; 

    std::cout << unescape(foo2) << std::endl;
}