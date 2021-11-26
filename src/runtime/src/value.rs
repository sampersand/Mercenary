use std::{
    borrow::{Borrow, Cow},
    cell::RefCell,
    cmp::Ordering,
    fmt::Debug,
    hash::Hash,
    ops::Deref,
    rc::Rc,
};

use crate::{instruction::Instruction, runtime::Runtime};

#[derive(Clone, Debug)]
pub enum Value {
    Null,
    String(String),
    Boolean(bool),
    Integer(i64),
    List(Rc<RefCell<Vec<Value>>>),
    Function(Function),
}

#[derive(Clone, Hash, PartialEq, Eq, Debug)]
pub enum Function {
    Bytecode(BytecodeFunction),
    Native(NativeFunction),
}

impl Function {
    pub fn get_local(&self, local_idx: u64) -> Value {
        match self {
            Function::Bytecode(bytecode) => bytecode.get_local(local_idx),
            Function::Native(_) => Value::Null,
        }
    }

    pub fn set_local(&mut self, local_idx: u64, val: Value) {
        if let Self::Bytecode(bytecode) = self {
            bytecode.set_local(local_idx, val)
        }
    }

    pub fn name(&self) -> &str {
        match self {
            Function::Bytecode(bytecode) => &bytecode.name,
            Function::Native(native) => native.name.borrow(),
        }
    }

    pub fn arity(&self) -> u64 {
        match self {
            Function::Bytecode(bytecode) => bytecode.arity,
            Function::Native(native) => native.arity,
        }
    }

    pub fn to_string(&self) -> String {
        let mut s = String::new();
        s += self.name();
        s += "(_0";
        for i in 1..self.arity() {
            s += &format!(", _{}", i);
        }

        match self {
            Function::Bytecode(_) => s += ") { /* bytecode */ }",
            Function::Native(_) => s += ") { /* machine code */ }",
        }

        s
    }

    pub fn is_bytecode(&self) -> bool {
        matches!(self, &Function::Bytecode(_))
    }
}

#[derive(Clone)]
pub struct NativeFunction {
    pub name: Cow<'static, str>,
    pub arity: u64,
    pub fun_ptr: fn(&mut Runtime) -> (),
}

impl Debug for NativeFunction {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("NativeFunction")
            .field("name", &self.name)
            .field("arity", &self.arity)
            .finish()
    }
}

impl Hash for NativeFunction {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        self.name.hash(state);
    }
}

impl PartialEq for NativeFunction {
    fn eq(&self, other: &Self) -> bool {
        self.name == other.name
    }
}

impl Eq for NativeFunction {}

#[derive(Clone, Debug)]
pub struct BytecodeFunction {
    pub name: String,
    pub arity: u64,
    pub code: Block,
    pub locals: Vec<Value>,
}

impl Hash for BytecodeFunction {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        self.name.hash(state);
    }
}

impl PartialEq for BytecodeFunction {
    fn eq(&self, other: &Self) -> bool {
        self.name == other.name
    }
}

impl Eq for BytecodeFunction {}

impl BytecodeFunction {
    pub fn get_local(&self, local_idx: u64) -> Value {
        self.locals
            .get(local_idx as usize)
            .map(|v| v.clone())
            .unwrap_or(Value::Null)
    }

    pub fn set_local(&mut self, local_idx: u64, val: Value) {
        while self.locals.len() < (local_idx + 1) as usize {
            self.locals.push(Value::Null);
        }

        self.locals[local_idx as usize] = val;
    }
}

#[derive(Clone, Default, Debug)]
pub struct Block(pub Vec<Instruction>);

impl Value {
    pub fn kindof(&self) -> Self {
        use Value::String;
        match self {
            Value::Null => String("null".into()),
            Value::String(_) => String("string".into()),
            Value::Boolean(_) => String("boolean".into()),
            Value::Integer(_) => String("integer".into()),
            Value::List(_) => String("array".into()),
            Value::Function(_) => String("$$function##".into()),
        }
    }

    pub fn length(&self) -> Self {
        match self {
            Value::String(s) => Value::Integer(s.len() as i64),
            Value::List(ary) => Value::Integer(RefCell::borrow(ary).len() as i64),
            Value::Function(func) => Value::Integer(func.arity() as i64),
            Value::Null => Value::Integer(0),
            _ => Value::Integer(1),
        }
    }

    pub fn truthy(&self) -> bool {
        match self {
            Value::String(s) => !s.is_empty(),
            Value::Boolean(b) => *b,
            Value::Integer(i) => *i != 0,
            Value::Null => false,
            Value::List(v) => !RefCell::borrow(v).is_empty(),
            Value::Function(_) => true,
        }
    }

    pub fn negate(&self) -> Self {
        match self {
            Value::Null => Value::Null,
            Value::String(s) => {
                let mut reversed = String::with_capacity(s.len());

                for ch in s.chars().rev() {
                    reversed.push(ch);
                }

                Value::String(reversed)
            }
            Value::Boolean(b) => Value::Boolean(!b),
            Value::Integer(i) => Value::Integer(-i),
            Value::List(l) => {
                let mut reversed = Vec::with_capacity(RefCell::borrow(&l).len());

                let l = RefCell::borrow(&l);

                for val in l.iter().rev() {
                    reversed.push(val.negate());
                }

                Value::List(Rc::new(RefCell::new(reversed)))
            }
            Value::Function(_) => Value::Boolean(false),
        }
    }

    pub fn add(&self, other: &Value) -> Value {
        use Value::*;
        match (self, other) {
            (Integer(a), Integer(b)) => Integer(*a + *b),
            (Integer(a), Boolean(b)) => Integer(*a + (*b as i64)),
            (Boolean(a), Integer(b)) => Integer((*a as i64) + *b),
            (Integer(a), Null) => Integer(*a),
            (Null, Integer(a)) => Integer(*a),
            (Boolean(a), Boolean(b)) => Integer((*a as i64) + (*b as i64)),
            (Boolean(a), Null) => Boolean(*a),
            (Null, Boolean(a)) => Boolean(*a),
            (String(a), String(b)) => String(format!("{}{}", a, b)),
            (String(a), Integer(b)) => String(format!("{}{}", a, b)),
            (Integer(a), String(b)) => String(format!("{}{}", a, b)),
            (String(a), Null) => String(format!("{}null", a)),
            (Null, String(a)) => String(format!("null{}", a)),
            (Boolean(a), String(b)) => String(format!("{}{}", a.to_string(), b)),
            (String(a), Boolean(b)) => String(format!("{}{}", a, b.to_string())),
            (List(l), List(m)) => {
                let mut new_l = RefCell::borrow(l).clone();
                new_l.extend_from_slice(RefCell::borrow(m).deref());
                List(Rc::new(RefCell::new(new_l)))
            }
            (List(l), val) => {
                let mut new_l = RefCell::borrow(l).clone();
                new_l.push(val.clone());
                List(Rc::new(RefCell::new(new_l)))
            }
            (val, List(l)) => {
                let mut new_l = RefCell::borrow(l).clone();
                new_l.push(val.clone());
                List(Rc::new(RefCell::new(new_l)))
            }
            (Null, Null) => Null,
            (Function(_), _) => Null,
            (_, Function(_)) => Null,
        }
    }

    pub fn subtraction(&self, other: &Value) -> Value {
        use Value::*;
        match (self, other) {
            (Integer(a), Integer(b)) => Integer(*a - *b),
            (Integer(a), Boolean(b)) => Integer(*a - (*b as i64)),
            (Boolean(a), Integer(b)) => Integer((*a as i64) - *b),
            _ => Null,
        }
    }

    pub fn multiply(&self, other: &Value) -> Value {
        use Value::*;
        match (self, other) {
            (Integer(a), Integer(b)) => Integer(*a * *b),
            (Integer(count), String(string)) | (String(string), Integer(count)) => {
                let mut repeated = std::string::String::with_capacity(string.len());
                for _ in 0..*count {
                    repeated.push_str(string);
                }

                String(repeated)
            }
            (List(l), Integer(times)) => {
                let mut to_extend = RefCell::borrow(&l).to_owned();
                let len = to_extend.len();
                for _ in 1..*times {
                    to_extend.extend_from_within(0..len)
                }
                Value::List(Rc::new(RefCell::new(to_extend)))
            }
            _ => Null,
        }
    }

    pub fn divide(&self, other: &Value) -> Value {
        use Value::*;
        match (self, other) {
            (Integer(a), Integer(b)) => {
                if *b == 0 {
                    String("∞".into())
                } else {
                    Integer(*a / *b)
                }
            }
            _ => Null,
        }
    }

    pub fn modulo(&self, other: &Value) -> Value {
        use Value::*;
        match (self, other) {
            (Integer(a), Integer(b)) => {
                if *b == 0 {
                    String("oopsie ><".into())
                } else {
                    Integer(*a % *b)
                }
            }
            _ => Null,
        }
    }

    pub fn compare(&self, other: &Self) -> Option<Ordering> {
        use Value::*;
        match (self, other) {
            (Integer(a), Integer(b)) => a.partial_cmp(b),
            (String(a), String(b)) => a.partial_cmp(b),
            (Integer(a), String(b)) => a.to_string().partial_cmp(b),
            (String(a), Integer(b)) => a.partial_cmp(&b.to_string()),
            (Boolean(a), Boolean(b)) => a.partial_cmp(b),
            (Boolean(a), Integer(b)) => (*a as i64).partial_cmp(b),
            (Integer(a), Boolean(b)) => a.partial_cmp(&(*b as i64)),
            (Boolean(a), String(b)) => a.partial_cmp(&b.is_empty()),
            (String(a), Boolean(b)) => a.is_empty().partial_cmp(b),
            (Null, Null) => Some(Ordering::Equal),
            (Null, _) => None,
            (_, Null) => None,
            (List(l), List(m)) => {
                // Inspired by https://doc.rust-lang.org/src/core/slice/cmp.rs.html#103-120
                let l = RefCell::borrow(&l);
                let m = RefCell::borrow(&m);
                for (a, b) in l.iter().zip(m.iter()) {
                    match a.compare(&b) {
                        Some(Ordering::Equal) => {}
                        non_eq => return non_eq,
                    }
                }

                l.len().partial_cmp(&m.len())
            }
            (List(_), _) => None,
            (_, List(_)) => None,
            (Function(fna), Function(fnb)) => match (fna, fnb) {
                (self::Function::Bytecode(a), self::Function::Bytecode(b)) => {
                    a.name.partial_cmp(&b.name)
                }
                (self::Function::Bytecode(_), self::Function::Native(_)) => None,
                (self::Function::Native(_), self::Function::Bytecode(_)) => None,
                (self::Function::Native(a), self::Function::Native(b)) => {
                    a.name.partial_cmp(&b.name)
                }
            },
            (Function(_), _) => None,
            (_, Function(_)) => None,
        }
    }

    pub fn to_string(&self) -> String {
        match self {
            Value::String(s) => s.clone(),
            Value::Boolean(b) => b.to_string(),
            Value::Integer(i) => i.to_string(),
            Value::Null => "null".into(),
            Value::List(v) => {
                let v = RefCell::borrow(v);
                if v.is_empty() {
                    return "[]".into();
                } else {
                    let mut s = format!("[{}", v[0].to_string());
                    for val in v.iter().skip(1) {
                        s += &format!(", {}", val.to_string());
                    }
                    s += "]";
                    s
                }
            }
            Value::Function(f) => f.to_string(),
        }
    }

    pub fn to_integer(&self) -> i64 {
        match self {
            Value::Null => 0,
            Value::String(s) => s.trim().parse::<i64>().unwrap_or(0),
            Value::Boolean(a) => *a as i64,
            Value::Integer(a) => *a,
            Value::List(_) => 0,
            Value::Function(_) => 0,
        }
    }
}
