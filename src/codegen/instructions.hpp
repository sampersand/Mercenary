#ifndef INSTRUCTIONS_CODEGEN
#define INSTRUCTIONS_CODEGEN

#include <stdint.h>

#include <string>
#include <vector>
#include <variant>

#include "ast.hpp"

using string = std::string;

namespace codegen {
    struct Arity {
        uint64_t value;
    };

    struct LocalIndex {
        uint64_t value;
    };

    struct IIdentifier {
        string* value;
    };

    /*
     * Instructions
     */

    /*
     * Import
     */

    // [string] -> []
    struct IImport {};

    /*
     * Function stuff
     */

    // [block] -> []
    struct IFunc {
        Arity parm_count;
        IIdentifier ident;
    };

    // [???] -> ???
    struct StartBlock {};
    struct EndBlock {};

    // [any] -> ⊥
    struct IReturn {};

    // [...any] -> any
    struct CallKnown {
        Arity arg_count;
        IIdentifier ident;
    };

    // [...any, func] -> any
    struct CallUnknown {
        Arity arg_count;
    };

    /*
     * Constants
     */

    // [] -> null
    struct NullConst {};

    // [] -> bool
    struct BooleanConst {
        bool value;   
    };

    // [] -> int
    struct IntegerConst {
        int64_t value;
    };

    // [] -> string
    struct StringConst {
        string* value;
    };

    // [...any] -> any list
    struct ListConst {
        uint64_t value;
    };

    /*
     * Local operations
     */

    // [] -> [any] 
    struct GetLocal {
        LocalIndex index;
    };

    // [any] -> []
    struct SetLocal {
        LocalIndex index;
    };

    /*
     * Control Flow
     */

    // [any] -> []
    struct Drop {};

    // [bool, block, block] -> []
    struct IIf {};

    // [block] -> []
    struct Loop {};

    // [bool] -> ⊥
    struct BreakIf {};

    /*
     * Global stuff
     */

    // [string] -> []
    struct IGlobal {};

    /*
     * Free binding operations
     */
    
    // [string] -> any
    struct GetFree {};

    // [string, any] -> []
    struct SetFree {};

    /*
     * Special Built-ins
     */

    // ~[] : [list, int] -> any
    // ==[] : [any, list, int] -> []

    /*
     * Aliases
     */

    using Instruction = std::variant<
        IImport, IFunc, StartBlock,
        EndBlock, IReturn, CallKnown,
        CallUnknown, NullConst, BooleanConst,
        IntegerConst, StringConst,
        ListConst, GetLocal, SetLocal,
        Drop, IIf, Loop, BreakIf,
        IGlobal, GetFree, SetFree
    >;

    using Instructions = std::vector<Instruction>;

    Instructions instructionify(const IndexAST&);

    string intructions_to_string(const Instructions&);
}

#endif