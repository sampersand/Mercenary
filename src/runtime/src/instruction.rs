use crate::value::Block;

#[derive(Clone, Debug)]
pub enum Instruction {
    Import,
    DefineFunction {
        param_count: u64,
        identifier: String,
    },
    StartBlock,
    EndBlock,
    Return,
    CallKnownFunction {
        arg_count: u64,
        identifier: String,
    },
    CallUnknownFunction {
        arg_count: u64,
    },
    NullConst,
    BooleanConst(bool),
    IntegerConst(i64),
    StringConst(String),
    ListCount {
        count: u64,
    },
    GetLocal {
        local_idx: u64,
    },
    SetLocal {
        local_idx: u64,
    },
    Drop,
    If {
        then: Block,
        else_: Block,
    },
    Loop {
        block: Block,
    },
    /// [21:17] RazzBerry: Oh BreakIf is a misnomer
    ///
    /// It should be BreakIfNot
    ///
    ///[21:17] (one, day, tuples, will, rule): bruh
    BreakIfNot,
    Global,
    GetFree,
    SetFree,
}
