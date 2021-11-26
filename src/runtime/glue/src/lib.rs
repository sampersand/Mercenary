mod ctypes;

use std::{
    error::Error,
    ffi::{CStr, CString},
    os::raw::c_char,
};

use runtime::{instruction::Instruction, value::Block};

use tracing::warn;

extern "C" {
    fn MercenaryGetInstructionFromString(source: *const c_char, len: u32) -> ctypes::Instructions;
    fn MercenaryFreeInstructions(insns: ctypes::Instructions);
}

pub fn parse_instructions_from_buf(buf: &[u8]) -> Result<Vec<Instruction>, Box<dyn Error>> {
    let cstring = CString::new(buf)?;

    let raw_insns = unsafe {
        MercenaryGetInstructionFromString(cstring.as_ptr(), libc::strlen(cstring.as_ptr()) as u32)
    };

    let mut insns = vec![];
    for i in 0..raw_insns.size {
        let raw_insn = unsafe { *raw_insns.insns.add(i as usize) };
        match raw_insn.tag {
            ctypes::IIMPORT => insns.push(Instruction::Import),
            ctypes::IFUNC => {
                let param_count = unsafe { raw_insn.insn.ifunc.parm_count };
                let identifier = unsafe { CStr::from_ptr(raw_insn.insn.ifunc.ident) }
                    .to_string_lossy()
                    .into_owned();
                insns.push(Instruction::DefineFunction {
                    param_count,
                    identifier,
                });
            }
            ctypes::START_BLOCK => insns.push(Instruction::StartBlock),
            ctypes::END_BLOCK => insns.push(Instruction::EndBlock),
            ctypes::IRETURN => insns.push(Instruction::Return),
            ctypes::CALL_KNOWN => {
                let arg_count = unsafe { raw_insn.insn.call_known.arg_count };
                let identifier = unsafe { CStr::from_ptr(raw_insn.insn.call_known.ident) }
                    .to_string_lossy()
                    .into_owned();
                tracing::trace!("glue: {}", identifier);
                insns.push(Instruction::CallKnownFunction {
                    arg_count,
                    identifier,
                });
            }
            ctypes::CALL_UNKNOWN => {
                let arg_count = unsafe { raw_insn.insn.call_unknown.arg_count };
                insns.push(Instruction::CallUnknownFunction { arg_count });
            }
            ctypes::NULL_CONST => insns.push(Instruction::NullConst),
            ctypes::BOOLEAN_CONST => {
                let value = unsafe { raw_insn.insn.boolean_const.value };
                insns.push(Instruction::BooleanConst(value));
            }
            ctypes::INTEGER_CONST => {
                let value = unsafe { raw_insn.insn.integer_const.value };
                insns.push(Instruction::IntegerConst(value));
            }
            ctypes::STRING_CONST => {
                let value = unsafe { CStr::from_ptr(raw_insn.insn.string_const.value) }
                    .to_string_lossy()
                    .into_owned();
                insns.push(Instruction::StringConst(value));
            }
            ctypes::LIST_CONST => {
                let count = unsafe { raw_insn.insn.list_const.value };
                insns.push(Instruction::ListCount { count })
            }
            ctypes::GET_LOCAL => {
                let local_idx = unsafe { raw_insn.insn.get_local.idx };
                insns.push(Instruction::GetLocal { local_idx });
            }
            ctypes::SET_LOCAL => {
                let local_idx = unsafe { raw_insn.insn.set_local.idx };
                insns.push(Instruction::SetLocal { local_idx });
            }
            ctypes::DROP => insns.push(Instruction::Drop),
            ctypes::IIF => insns.push(Instruction::If {
                then: Block::default(),
                else_: Block::default(),
            }),
            ctypes::LOOP => insns.push(Instruction::Loop {
                block: Block::default(),
            }),
            // See doc comment on Instruction::BreakIfNot, too lazy to update the glue
            ctypes::BREAK_IF => insns.push(Instruction::BreakIfNot),
            ctypes::IGLOBAL => insns.push(Instruction::Global),
            ctypes::GET_FREE => insns.push(Instruction::GetFree),
            ctypes::SET_FREE => insns.push(Instruction::SetFree),
            unk => warn!("Found unknow raw insn tag: {}", unk),
        }
    }

    unsafe { MercenaryFreeInstructions(raw_insns) };

    Ok(insns)
}
