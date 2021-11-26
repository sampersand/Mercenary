use crate::{
    operators::{binary, ternary, unary},
    value::NativeFunction,
};

pub const INTRINSICS: &[NativeFunction] = &[
    ternary::INDEX_SET,
    binary::EQUAL,
    binary::NOT_EQUAL,
    binary::LESS_THAN,
    binary::LESS_THAN_EQUAL,
    binary::GREATER_THAN,
    binary::GREATER_THAN_OR_EQUAL,
    binary::AND,
    binary::OR,
    binary::ADD,
    binary::SUB,
    binary::MULTIPLY,
    binary::DIVIDE,
    binary::MODULO,
    binary::INDEX,
    unary::NEGATE,
    unary::NOT,
    funcs::PRINT,
    funcs::EXIT,
    funcs::ITOA,
    funcs::ATOI,
    funcs::KINDOF,
    funcs::LENGTH,
    funcs::INSERT,
    funcs::PROMPT,
    funcs::DELETE,
    funcs::RANDOM,
    funcs::SUBSTR,
    funcs::DUMP,
];

pub mod funcs {
    use std::{
        borrow::Cow,
        cell::RefCell,
        io::{self, Write},
    };

    use rand::Rng;
    use tracing::error;

    use crate::{
        runtime::Runtime,
        value::{NativeFunction, Value},
    };

    pub const PRINT: NativeFunction = NativeFunction {
        name: Cow::Borrowed("print"),
        arity: 1,
        fun_ptr: print,
    };

    pub const PROMPT: NativeFunction = NativeFunction {
        name: Cow::Borrowed("prompt"),
        arity: 0,
        fun_ptr: prompt,
    };

    pub const EXIT: NativeFunction = NativeFunction {
        name: Cow::Borrowed("exit"),
        arity: 1,
        fun_ptr: exit,
    };

    pub const ITOA: NativeFunction = NativeFunction {
        name: Cow::Borrowed("itoa"),
        arity: 1,
        fun_ptr: itoa,
    };

    pub const ATOI: NativeFunction = NativeFunction {
        name: Cow::Borrowed("atoi"),
        arity: 1,
        fun_ptr: atoi,
    };

    pub const KINDOF: NativeFunction = NativeFunction {
        name: Cow::Borrowed("kindof"),
        arity: 1,
        fun_ptr: kindof,
    };

    pub const LENGTH: NativeFunction = NativeFunction {
        name: Cow::Borrowed("length"),
        arity: 1,
        fun_ptr: length,
    };

    pub const INSERT: NativeFunction = NativeFunction {
        name: Cow::Borrowed("insert"),
        arity: 3,
        fun_ptr: insert,
    };

    pub const DELETE: NativeFunction = NativeFunction {
        name: Cow::Borrowed("delete"),
        arity: 2,
        fun_ptr: delete,
    };

    pub const SUBSTR: NativeFunction = NativeFunction {
        name: Cow::Borrowed("substr"),
        arity: 3,
        fun_ptr: substr,
    };

    pub const RANDOM: NativeFunction = NativeFunction {
        name: Cow::Borrowed("random"),
        arity: 0,
        fun_ptr: random,
    };

    pub const DUMP: NativeFunction = NativeFunction {
        name: Cow::Borrowed("dump"),
        arity: 1,
        fun_ptr: dump,
    };

    fn dump(runtime: &mut Runtime) {
        let a = runtime.pop_value_from_stack();

        println!("{:#?}", a);
    }

    fn itoa(runtime: &mut Runtime) {
        let a = runtime.pop_value_from_stack();

        runtime.push_value_to_stack(Value::String(a.to_string()))
    }

    fn atoi(runtime: &mut Runtime) {
        let a = runtime.pop_value_from_stack();

        runtime.push_value_to_stack(Value::Integer(a.to_integer()))
    }

    fn kindof(runtime: &mut Runtime) {
        let a = runtime.pop_value_from_stack();

        runtime.push_value_to_stack(a.kindof())
    }

    fn length(runtime: &mut Runtime) {
        let a = runtime.pop_value_from_stack();

        runtime.push_value_to_stack(a.length())
    }

    fn print(runtime: &mut Runtime) {
        let a = runtime.pop_value_from_stack();

        print!("{}", a.to_string());
        let _ = io::stdout().lock().flush();
    }

    fn prompt(runtime: &mut Runtime) {
        let mut buf = String::new();
        let res = io::stdin().read_line(&mut buf);

        if let Err(why) = res {
            error!("Prompt got a {}", why);
        }

        runtime.push_value_to_stack(Value::String(buf));
    }

    fn exit(runtime: &mut Runtime) {
        let a = runtime.pop_value_from_stack();

        std::process::exit(a.to_integer() as i32)
    }

    fn insert(runtime: &mut Runtime) {
        let value = runtime.pop_value_from_stack();
        let index = runtime.pop_value_from_stack().to_integer();
        let list = runtime.pop_value_from_stack();

        match list {
            Value::List(l) => {
                let mut l = RefCell::borrow_mut(&l);
                if index as usize >= l.len() {
                    l.resize((index + 1) as usize, Value::Null);
                }

                l.insert(index as usize, value);
            }
            not_a_list => error!("Called insert on not a list: {:?}", not_a_list),
        };
    }

    fn delete(runtime: &mut Runtime) {
        let index = runtime.pop_value_from_stack().to_integer();
        let list = runtime.pop_value_from_stack();

        match list {
            Value::List(list) => {
                let mut list = RefCell::borrow_mut(&list);
                if (index as usize) < list.len() {
                    list.remove(index as usize);
                }
            }
            _ => {}
        }
    }

    fn substr(runtime: &mut Runtime) {
        let length = runtime.pop_value_from_stack().to_integer() as usize;
        let start = runtime.pop_value_from_stack().to_integer() as usize;
        let string = runtime.pop_value_from_stack().to_string();

        runtime.push_value_to_stack(Value::String(string[start..][..length].into()));
    }

    fn random(runtime: &mut Runtime) {
        runtime.push_value_to_stack(Value::Integer(rand::thread_rng().gen_range(0..=i64::MAX)));
    }
}
