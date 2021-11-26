#include <cstring>
#include <variant>

// Not my code, not my warnings!
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

extern "C" {
#include "../../../lexer/lexer.h"
#include "../../../parser/parser.h"
}


#include "../../../codegen/ast.hpp"
#include "../../../codegen/instructions.hpp"
#include "../../../codegen/middle_end.hpp"

#pragma GCC diagnostic pop

#define IIMPORT 0
#define IFUNC 1
#define START_BLOCK 2
#define END_BLOCK 3
#define IRETURN 4
#define CALL_KNOWN 5
#define CALL_UNKNOWN 6
#define NULL_CONST 7
#define BOOLEAN_CONST 8
#define INTEGER_CONST 9
#define STRING_CONST 10
#define LIST_CONST 11
#define GET_LOCAL 12
#define SET_LOCAL 13
#define DROP 14
#define IIF 15
#define LOOP 16
#define BREAK_IF 17
#define IGLOBAL 18
#define GET_FREE 19
#define SET_FREE 20

struct IFunc {
    uint64_t parm_count;
    char const* ident;
};

struct CallKnown {
    uint64_t arg_count;
    char const* ident;
};

struct CallUnknown {
    uint64_t arg_count;
};

struct BooleanConst {
    bool value;   
};

struct IntegerConst {
    int64_t value;
};

struct StringConst {
    char const* value;
};

struct ListConst {
    uint64_t value;
};

struct GetLocal {
    uint64_t index;
};

struct SetLocal {
    uint64_t index;
};


union Instruction {
    void const* dummy;
    IFunc ifunc;
    CallKnown call_known;
    CallUnknown call_unknown;
    BooleanConst boolean_const;
    IntegerConst integer_const;
    StringConst string_const;
    ListConst list_const;
    GetLocal get_local;
    SetLocal set_local;
};

struct InstructionAndTag {
    Instruction insn;
    uint8_t tag;
};

struct Instructions {
    InstructionAndTag* insns;
    uint32_t size;
};

static auto MercenaryTranslateCodegenInstructionsToGoodInstructions(codegen::Instructions Insns) noexcept -> Instructions;

extern "C" auto MercenaryFreeInstructions(Instructions Insns) noexcept -> void {
    for (uint32_t i = 0; i < Insns.size; i++) {
        if (Insns.insns[i].tag == STRING_CONST) delete Insns.insns[i].insn.string_const.value;
        if (Insns.insns[i].tag == CALL_KNOWN) delete Insns.insns[i].insn.call_known.ident;
        if (Insns.insns[i].tag == IFUNC) delete Insns.insns[i].insn.ifunc.ident;
    }
    delete Insns.insns;
}

extern "C" auto MercenaryGetInstructionFromString(char const* Source, uint32_t Length) -> Instructions {
    program_t program;

    eh_data_t eh = {
        .stream_start = Source,
        .overall_len = Length,
        .line_offsets = mk_offsets_list(Source, Length)
    };

    pres_t res = parse_program(&Source, &program, eh);

    if (!res) {
        fprintf(stderr, "[GLUE] Parsing failed");
        std::abort();
    }

    codegen::StringAST const ast = codegen::to_cpp_ast(&program);
    codegen::IndexAST const iast = codegen::de_bruijnify(ast);
    codegen::Instructions const insns = codegen::instructionify(iast);

    return MercenaryTranslateCodegenInstructionsToGoodInstructions(std::move(insns));
}

struct Visitor {
    auto operator()(codegen::IImport const&) {
        return InstructionAndTag {
            .insn = Instruction { .dummy = nullptr, },
            .tag = IIMPORT,
        };
    }

    auto operator()(codegen::IFunc const& ifunc) {
        auto size = ifunc.ident.value->size();
        auto* string = new char[size + 1];
        std::memcpy(string, ifunc.ident.value->data(), ifunc.ident.value->size());
        string[size] = '\0';

        return InstructionAndTag {
            .insn = Instruction { .ifunc = IFunc {
                .parm_count = ifunc.parm_count.value,
                .ident = string,
            }},
            .tag = IFUNC,
        };
    }

    auto operator()(codegen::StartBlock const&) {
        return InstructionAndTag {
            .insn = Instruction { .dummy = nullptr, },
            .tag = START_BLOCK,
        };
    }

    auto operator()(codegen::EndBlock const&) {
        return InstructionAndTag {
            .insn = Instruction { .dummy = nullptr, },
            .tag = END_BLOCK,
        };
    }

    auto operator()(codegen::IReturn const&) {
        return InstructionAndTag {
            .insn = Instruction { .dummy = nullptr, },
            .tag = IRETURN,
        };
    }

    auto operator()(codegen::CallKnown const& ck) {
        auto str_size = ck.ident.value->size();
        auto* ident = new char[str_size + 1];
        std::memcpy(ident, ck.ident.value->data(), ck.ident.value->size());
        ident[str_size] = '\0';

        return InstructionAndTag {
            .insn = Instruction { .call_known = CallKnown {
                .arg_count = ck.arg_count.value,
                .ident = ident,
            }},
            .tag = CALL_KNOWN,
        };
    }

    auto operator()(codegen::CallUnknown const& call_unknown) {
        return InstructionAndTag {
            .insn = Instruction { .call_unknown = CallUnknown {
                .arg_count = call_unknown.arg_count.value
            }},
            .tag = CALL_UNKNOWN,
        };
    }

    auto operator()(codegen::NullConst const&) {
        return InstructionAndTag {
            .insn = Instruction { .dummy = nullptr, },
            .tag = NULL_CONST,
        };
    }

    auto operator()(codegen::BooleanConst const& bc) {
        return InstructionAndTag {
            .insn = Instruction { .boolean_const = BooleanConst { .value = bc.value } },
            .tag = BOOLEAN_CONST,
        };
    }

    auto operator()(codegen::IntegerConst const& ic) {
        return InstructionAndTag {
            .insn = Instruction { .integer_const = IntegerConst { .value = ic.value } },
            .tag = INTEGER_CONST,
        };
    }
    
    auto operator()(codegen::StringConst const& sc) {
        auto size = sc.value->size();
        auto* string_copy = new char[size];
        std::memcpy(string_copy, sc.value->data(), sc.value->size());
        string_copy[size] = '\0';

        return InstructionAndTag {
            .insn = Instruction { .string_const = StringConst { .value = string_copy } },
            .tag = STRING_CONST,
        };

    }
    
    auto operator()(codegen::ListConst const& lc) {
        return InstructionAndTag {
            .insn = Instruction { .list_const = ListConst { .value = lc.value }},
            .tag = LIST_CONST,
        };
    }
    
    auto operator()(codegen::GetLocal const& gl) {
        return InstructionAndTag {
            .insn = Instruction { .get_local = GetLocal { .index = gl.index.value } },
            .tag = GET_LOCAL,
        };
    }
    
    auto operator()(codegen::SetLocal const& sl) {
        return InstructionAndTag {
            .insn = Instruction { .set_local = SetLocal { .index = sl.index.value } },
            .tag = SET_LOCAL,
        };
    }
    
    auto operator()(codegen::Drop const&) {
        return InstructionAndTag {
            .insn = Instruction { .dummy = nullptr, },
            .tag = DROP,
        };
    }
    
    auto operator()(codegen::IIf const&) {
        return InstructionAndTag {
            .insn = Instruction { .dummy = nullptr, },
            .tag = IIF,
        };
    }
    
    auto operator()(codegen::Loop const&) {
        return InstructionAndTag {
            .insn = Instruction { .dummy = nullptr, },
            .tag = LOOP,
        };
    }
    
    auto operator()(codegen::BreakIf const&) {
        return InstructionAndTag {
            .insn = Instruction { .dummy = nullptr, },
            .tag = BREAK_IF,
        };
    }
    
    auto operator()(codegen::IGlobal const&) {
        return InstructionAndTag {
            .insn = Instruction { .dummy = nullptr, },
            .tag = IGLOBAL,
        };
    }
    
    auto operator()(codegen::GetFree const&) {
        return InstructionAndTag {
            .insn = Instruction { .dummy = nullptr, },
            .tag = GET_FREE,
        };
    }
    
    auto operator()(codegen::SetFree const&) {
        return InstructionAndTag {
            .insn = Instruction { .dummy = nullptr, },
            .tag = SET_FREE,
        };
    }
};

static auto MercenaryTranslateCodegenInstructionsToGoodInstructions(codegen::Instructions Insns) noexcept -> Instructions try {
    auto* insns = new InstructionAndTag[Insns.size()];
    uint32_t k = 0;
    for (auto const& insn : Insns) {
        auto goodInsn = std::visit(Visitor {}, insn);
        insns[k++] = std::move(goodInsn);
    }

    return Instructions {
        .insns = insns,
        .size = k,
    };
} catch (std::bad_alloc const& ex) {
    std::fprintf(stderr, "[GLUE] plz buy more wam");
    std::abort();
} catch (std::exception const& ex) {
    std::fprintf(stderr, "[GLUE] got an exception oooo: %s", ex.what());
    std::abort();
} catch (...) {
    std::fprintf(stderr, "[GLUE] I blame it on the codegen guy");
    std::abort();
}
