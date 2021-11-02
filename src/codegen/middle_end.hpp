#ifndef MIDDLE_END_CODEGEN
#define MIDDLE_END_CODEGEN

#include "ast.hpp"

namespace codegen {
    IndexAST de_bruijnify(const StringAST&);
}

#endif