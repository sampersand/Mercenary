use std::{ffi::c_void, os::raw::c_char};

// the C stands for codegen

#[repr(C)]
#[derive(Clone, Copy)]
pub struct Instructions {
    pub insns: *mut InstructionAndTag,
    pub size: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct InstructionAndTag {
    pub insn: Instruction,
    pub tag: u8,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub union Instruction {
    pub dummy: *const c_void,
    pub ifunc: IFunc,
    pub call_known: CallKnown,
    pub call_unknown: CallUnknown,
    pub boolean_const: BooleanConst,
    pub integer_const: IntegerConst,
    pub string_const: StringConst,
    pub list_const: ListConst,
    pub get_local: GetLocal,
    pub set_local: SetLocal,
}

pub const IIMPORT: u8 = 0;
pub const IFUNC: u8 = 1;
pub const START_BLOCK: u8 = 2;
pub const END_BLOCK: u8 = 3;
pub const IRETURN: u8 = 4;
pub const CALL_KNOWN: u8 = 5;
pub const CALL_UNKNOWN: u8 = 6;
pub const NULL_CONST: u8 = 7;
pub const BOOLEAN_CONST: u8 = 8;
pub const INTEGER_CONST: u8 = 9;
pub const STRING_CONST: u8 = 10;
pub const LIST_CONST: u8 = 11;
pub const GET_LOCAL: u8 = 12;
pub const SET_LOCAL: u8 = 13;
pub const DROP: u8 = 14;
pub const IIF: u8 = 15;
pub const LOOP: u8 = 16;
pub const BREAK_IF: u8 = 17;
pub const IGLOBAL: u8 = 18;
pub const GET_FREE: u8 = 19;
pub const SET_FREE: u8 = 20;

#[repr(C)]
#[derive(Clone, Copy)]
pub struct IFunc {
    pub parm_count: u64,
    pub ident: *const c_char,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct CallKnown {
    pub arg_count: u64,
    pub ident: *const c_char,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct CallUnknown {
    pub arg_count: u64,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct BooleanConst {
    pub value: bool,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct IntegerConst {
    pub value: i64,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct StringConst {
    pub value: *const c_char,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct ListConst {
    pub value: u64,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct GetLocal {
    pub idx: u64,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct SetLocal {
    pub idx: u64,
}
