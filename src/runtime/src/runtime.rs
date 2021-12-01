use std::{
    cell::RefCell,
    collections::HashSet,
    error::Error,
    mem,
    path::{Path, PathBuf},
    rc::Rc,
    slice::Iter,
};

use crate::{
    instruction::Instruction,
    value::{Block, BytecodeFunction, Function, NativeFunction, Value},
};

use tracing::error;

pub struct Runtime {
    pub(crate) value_stack: Vec<Value>,
    globals: Vec<(String, Value)>,
    functions: HashSet<Function>,
    pub(crate) function_stack: Vec<Function>,
    block_stack: Vec<Block>,
    instruction_reader: InstructionReader,
    argv: Value,
    base_path: PathBuf,
    pub(crate) return_value: Value,
}

type InstructionReader = Box<dyn Fn(&str, &Path) -> Result<Vec<Instruction>, Box<dyn Error>>>;

#[derive(PartialEq, Eq, Hash, Debug, Clone, Copy)]
pub enum BreakRequested {
    Return,
    Yes,
    No,
}

impl Runtime {
    pub fn create(
        instruction_reader: InstructionReader,
        intrinsics: &[NativeFunction],
        argv: Value,
        base_path: PathBuf,
    ) -> Self {
        let mut functions = HashSet::new();

        functions.reserve(intrinsics.len());
        for intrinsic in intrinsics {
            if !functions.insert(Function::Native(intrinsic.clone())) {
                error!(
                    "TO THE EMBEDDER OF THIS INTERPRETER: Intrinsics array has dupes, not cool bro"
                )
            }
        }

        Self {
            value_stack: vec![],
            globals: vec![],
            functions,
            function_stack: vec![Function::Bytecode(BytecodeFunction {
                name: "<top>".into(),
                arity: 0,
                code: Block::default(),
                locals: vec![],
            })],
            block_stack: vec![],
            instruction_reader,
            argv,
            base_path,
            return_value: Value::Null,
        }
    }

    pub fn execute_program(&mut self, insns: &[Instruction]) {
        self.execute_insns(insns);

        if let Some(func) = self.functions.iter().find(|f| f.name() == "main") {
            if func.is_bytecode() {
                let func = func.clone();
                self.value_stack.push(self.argv.clone());
                self.execute_function(func);
            }
        }
    }

    pub fn execute_function(&mut self, func: Function) {
        match func {
            Function::Bytecode(bytecode) => {
                let mut new_bytecode = bytecode.clone();
                for _ in 0..bytecode.arity {
                    let arg = self.pop_value_from_stack();
                    new_bytecode.locals.insert(0, arg);
                }
                self.function_stack.push(Function::Bytecode(new_bytecode));
                let old_value_stack_size = self.value_stack.len();
                self.execute_insns(&bytecode.code.0);
                let r#return = mem::replace(&mut self.return_value, Value::Null);
                while self.value_stack.len() > old_value_stack_size {
                    if self.value_stack.is_empty() {
                        break;
                    }
                    self.value_stack.pop();
                }
                self.push_value_to_stack(r#return);
            }
            Function::Native(native) => {
                (native.fun_ptr)(self);
            }
        }
    }

    pub fn execute_insns(&mut self, insns: &[Instruction]) -> BreakRequested {
        let mut insns_iter = insns.iter();
        'main: while let Some(insn) = insns_iter.next() {
            match insn {
                Instruction::Import => {
                    let path = self.value_stack.pop().map(|v| v.to_string()).unwrap();
                    let imported_insns =
                        (self.instruction_reader)(&path[1..][..path.len() - 2], &self.base_path);
                    self.execute_program(&imported_insns.unwrap())
                }
                Instruction::DefineFunction {
                    param_count: _,
                    identifier: _,
                } => { /* handled by build_block */ }
                Instruction::StartBlock => {
                    self.build_block(&mut insns_iter);
                }
                Instruction::EndBlock => {}
                Instruction::Return => {
                    self.return_value = self.value_stack.pop().unwrap_or(Value::Null);
                    self.function_stack.pop();
                    return BreakRequested::Return;
                }
                Instruction::CallKnownFunction {
                    arg_count,
                    identifier,
                } => {
                    let mut func = None;
                    for function in self.functions.iter() {
                        if function.name() == identifier {
                            func = Some(function.clone());
                            break;
                        }
                    }
                    let mut func = func.unwrap();

                    if let Function::Bytecode(func) = &mut func {
                        func.locals.reserve(*arg_count as usize);

                        for i in 0..*arg_count {
                            func.set_local(i, self.value_stack.pop().unwrap_or(Value::Null));
                        }
                    }

                    self.execute_function(func);
                }
                Instruction::CallUnknownFunction { .. } => {
                    let function = match self.value_stack.pop().unwrap() {
                        Value::Function(func) => func,
                        var => panic!(
                            "\n{}\n\nfunction_stack: {:#?}\n\n\nglobals: {:#?}",
                            var.to_string(),
                            self.function_stack,
                            self.globals
                        ),
                    };

                    self.execute_function(function);
                }
                Instruction::NullConst => self.value_stack.push(Value::Null),
                Instruction::BooleanConst(val) => self.value_stack.push(Value::Boolean(*val)),
                Instruction::IntegerConst(val) => self.value_stack.push(Value::Integer(*val)),
                Instruction::StringConst(val) => self.value_stack.push(Value::String(val.clone())),
                Instruction::ListCount { count } => {
                    let mut list = vec![];
                    let mut count_too_big = false;
                    for _ in 0..*count {
                        match self.value_stack.pop() {
                            Some(val) => list.insert(0, val),
                            None => count_too_big = true,
                        }
                    }

                    if count_too_big {
                        error!(
                            "Tried making a {} long list, but only {} values were on the stack",
                            count,
                            list.len()
                        );
                    }

                    self.value_stack
                        .push(Value::List(Rc::new(RefCell::new(list))));
                }
                Instruction::GetLocal { local_idx } => {
                    let value = self.function_stack.last().unwrap().get_local(*local_idx);
                    self.value_stack.push(value);
                }
                Instruction::SetLocal { local_idx } => {
                    self.function_stack.last_mut().unwrap().set_local(
                        *local_idx,
                        self.value_stack.last().cloned().unwrap_or(Value::Null),
                    )
                }
                Instruction::Drop => self.value_stack.pop().map_or((), |_| ()),
                Instruction::If { then, else_ } => {
                    let old_value_stack_size = self.value_stack.len();
                    let break_requested =
                        if self.value_stack.pop().map(|v| v.truthy()).unwrap_or(false) {
                            self.execute_insns(&then.0)
                        } else {
                            self.execute_insns(&else_.0)
                        };

                    while self.value_stack.len() >= old_value_stack_size {
                        if self.value_stack.is_empty() {
                            break;
                        }
                        self.value_stack.pop();
                    }

                    if break_requested != BreakRequested::No {
                        return break_requested;
                    }
                }
                Instruction::Loop { block } => loop {
                    let old_value_stack_size = self.value_stack.len();
                    let break_requested = self.execute_insns(&block.0);
                    if break_requested == BreakRequested::Yes {
                        while self.value_stack.len() >= old_value_stack_size {
                            if self.value_stack.is_empty() {
                                break;
                            }

                            self.value_stack.pop();
                        }
                        break;
                    }
                    while self.value_stack.len() >= old_value_stack_size {
                        if self.value_stack.is_empty() {
                            break;
                        }

                        self.value_stack.pop();
                    }
                    if break_requested == BreakRequested::Return {
                        return BreakRequested::Return;
                    }
                },
                Instruction::BreakIfNot => {
                    if !self.value_stack.pop().map(|v| v.truthy()).unwrap_or(true) {
                        return BreakRequested::Yes;
                    }
                }
                Instruction::Global => {
                    let ident = self.value_stack.pop().unwrap().to_string();
                    self.globals.push((ident, Value::Null));
                }
                Instruction::GetFree => {
                    let ident = self.value_stack.pop().map(|v| v.to_string());
                    match ident {
                        Some(ident) => {
                            'inner: for (name, global) in self.globals.iter() {
                                if name == &ident {
                                    if matches!(global, Value::Null) {
                                        break 'inner;
                                    }

                                    self.value_stack.push(global.clone());
                                    continue 'main;
                                }
                            }

                            let function = self
                                .functions
                                .iter()
                                .find(|f| f.name() == &ident)
                                .map(|f| Value::Function(f.clone()))
                                .unwrap_or(Value::Null);

                            self.value_stack.push(function);
                        }
                        None => self.value_stack.push(Value::Null),
                    }
                }
                Instruction::SetFree => {
                    let ident = self.value_stack.pop().map(|v| v.to_string());
                    let value = self.value_stack.pop().unwrap_or(Value::Null);

                    if let Some(ident) = ident {
                        for (name, global) in self.globals.iter_mut() {
                            if name == &ident {
                                *global = value;
                                continue 'main;
                            }
                        }
                        self.globals.push((ident, value));
                    }
                }
            }
        }

        BreakRequested::No
    }

    pub fn pop_value_from_stack(&mut self) -> Value {
        self.value_stack.pop().unwrap_or(Value::Null)
    }

    pub fn push_value_to_stack(&mut self, value: Value) {
        self.value_stack.push(value)
    }

    pub fn build_block(&mut self, insns_iter: &mut Iter<Instruction>) {
        self.block_stack.push(Block::default());

        while let Some(insn) = insns_iter.next() {
            if matches!(insn, &Instruction::StartBlock) {
                self.block_stack.push(Block::default())
            } else if let Instruction::If { then, else_ } = insn {
                let else_ = if else_.0.is_empty() {
                    self.block_stack.pop().unwrap()
                } else {
                    else_.clone()
                };

                let then = if then.0.is_empty() {
                    self.block_stack.pop().unwrap()
                } else {
                    then.clone()
                };

                self.block_stack
                    .last_mut()
                    .unwrap()
                    .0
                    .push(Instruction::If { then, else_ })
            } else if let Instruction::Loop { block } = insn {
                let block = if block.0.is_empty() {
                    self.block_stack.pop().unwrap()
                } else {
                    block.clone()
                };

                self.block_stack
                    .last_mut()
                    .unwrap()
                    .0
                    .push(Instruction::Loop { block });
            } else if let Instruction::DefineFunction {
                param_count,
                identifier,
            } = insn
            {
                let bytecode = BytecodeFunction {
                    name: identifier.clone(),
                    arity: *param_count,
                    code: self.block_stack.pop().unwrap(),
                    locals: vec![],
                };

                self.functions.insert(Function::Bytecode(bytecode));

                return;
            } else {
                self.block_stack
                    .last_mut()
                    .unwrap()
                    .0
                    .push(insn.clone().into());
            }
        }
    }
}
