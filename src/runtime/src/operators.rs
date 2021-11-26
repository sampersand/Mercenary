pub mod ternary {
    use std::{borrow::Cow, cell::RefCell};

    use crate::{
        runtime::Runtime,
        value::{NativeFunction, Value},
    };

    pub const INDEX_SET: NativeFunction = NativeFunction {
        name: Cow::Borrowed("==[]"),
        arity: 3,
        fun_ptr: index_set,
    };

    fn index_set(runtime: &mut Runtime) {
        let idx = runtime.pop_value_from_stack().to_integer() as usize;
        let list = runtime.pop_value_from_stack();
        let value = runtime.pop_value_from_stack();

        if let Value::List(list) = list {
            let mut list = RefCell::borrow_mut(&list);
            if list.len() < idx {
                list.resize(idx, Value::Null);
            }
            list[idx] = value;
        }
    }
}

pub mod binary {
    use std::{borrow::Cow, cmp::Ordering};

    use crate::{
        runtime::Runtime,
        value::{
            NativeFunction,
            Value::{self, Boolean},
        },
    };

    pub const EQUAL: NativeFunction = NativeFunction {
        name: Cow::Borrowed("~=="),
        arity: 2,
        fun_ptr: equal,
    };

    pub const NOT_EQUAL: NativeFunction = NativeFunction {
        name: Cow::Borrowed("~!="),
        arity: 2,
        fun_ptr: not_equal,
    };

    pub const LESS_THAN: NativeFunction = NativeFunction {
        name: Cow::Borrowed("~<"),
        arity: 2,
        fun_ptr: less_than,
    };

    pub const LESS_THAN_EQUAL: NativeFunction = NativeFunction {
        name: Cow::Borrowed("~<="),
        arity: 2,
        fun_ptr: less_than_or_equal,
    };

    pub const GREATER_THAN: NativeFunction = NativeFunction {
        name: Cow::Borrowed("~>"),
        arity: 2,
        fun_ptr: greater_than,
    };

    pub const GREATER_THAN_OR_EQUAL: NativeFunction = NativeFunction {
        name: Cow::Borrowed("~>="),
        arity: 2,
        fun_ptr: greater_than_or_equal,
    };

    pub const AND: NativeFunction = NativeFunction {
        name: Cow::Borrowed("~&&"),
        arity: 2,
        fun_ptr: and,
    };

    pub const OR: NativeFunction = NativeFunction {
        name: Cow::Borrowed("~||"),
        arity: 2,
        fun_ptr: or,
    };

    pub const ADD: NativeFunction = NativeFunction {
        name: Cow::Borrowed("~+"),
        arity: 2,
        fun_ptr: add,
    };

    pub const SUB: NativeFunction = NativeFunction {
        name: Cow::Borrowed("~-"),
        arity: 2,
        fun_ptr: sub,
    };

    pub const MULTIPLY: NativeFunction = NativeFunction {
        name: Cow::Borrowed("~*"),
        arity: 2,
        fun_ptr: multiply,
    };

    pub const DIVIDE: NativeFunction = NativeFunction {
        name: Cow::Borrowed("~/"),
        arity: 2,
        fun_ptr: divide,
    };

    pub const MODULO: NativeFunction = NativeFunction {
        name: Cow::Borrowed("~%"),
        arity: 2,
        fun_ptr: modulo,
    };

    pub const INDEX: NativeFunction = NativeFunction {
        name: Cow::Borrowed("~[]"),
        arity: 2,
        fun_ptr: index,
    };

    fn index(runtime: &mut Runtime) {
        let idx = runtime.pop_value_from_stack().to_integer() as usize;
        let list = runtime.pop_value_from_stack();

        if let Value::List(list) = list {
            let list = list.borrow();
            runtime.push_value_to_stack(list[idx].clone());
        } else if let Value::String(string) = list {
            runtime.push_value_to_stack(Value::String(string[idx..][..1].to_string()));
        }
    }

    fn equal(runtime: &mut Runtime) {
        let b = runtime.pop_value_from_stack();
        let a = runtime.pop_value_from_stack();

        let compare_result = a.compare(&b);

        runtime.push_value_to_stack(match compare_result {
            Some(ordering) => Boolean(ordering == Ordering::Equal),
            None => Boolean(false),
        })
    }

    fn not_equal(runtime: &mut Runtime) {
        let b = runtime.pop_value_from_stack();
        let a = runtime.pop_value_from_stack();

        let compare_result = a.compare(&b);

        runtime.push_value_to_stack(match compare_result {
            Some(ordering) => Boolean(ordering != Ordering::Equal),
            None => Boolean(true),
        })
    }

    fn greater_than(runtime: &mut Runtime) {
        let b = runtime.pop_value_from_stack();
        let a = runtime.pop_value_from_stack();

        let compare_result = a.compare(&b);

        runtime.push_value_to_stack(match compare_result {
            Some(ordering) => Boolean(ordering == Ordering::Greater),
            None => Boolean(false),
        })
    }

    fn greater_than_or_equal(runtime: &mut Runtime) {
        let b = runtime.pop_value_from_stack();
        let a = runtime.pop_value_from_stack();

        let compare_result = a.compare(&b);

        runtime.push_value_to_stack(match compare_result {
            Some(ordering) => Boolean(ordering == Ordering::Equal || ordering == Ordering::Greater),
            None => Boolean(false),
        })
    }

    fn less_than(runtime: &mut Runtime) {
        let b = runtime.pop_value_from_stack();
        let a = runtime.pop_value_from_stack();

        let compare_result = a.compare(&b);

        runtime.push_value_to_stack(match compare_result {
            Some(ordering) => Boolean(ordering == Ordering::Less),
            None => Boolean(false),
        })
    }

    fn less_than_or_equal(runtime: &mut Runtime) {
        let b = runtime.pop_value_from_stack();
        let a = runtime.pop_value_from_stack();

        let compare_result = a.compare(&b);

        runtime.push_value_to_stack(match compare_result {
            Some(ordering) => Boolean(ordering == Ordering::Less || ordering == Ordering::Equal),
            None => Boolean(false),
        })
    }

    fn and(runtime: &mut Runtime) {
        let b = runtime.pop_value_from_stack();
        let a = runtime.pop_value_from_stack();

        runtime.push_value_to_stack(Boolean(a.truthy() && b.truthy()))
    }

    fn or(runtime: &mut Runtime) {
        let b = runtime.pop_value_from_stack();
        let a = runtime.pop_value_from_stack();

        runtime.push_value_to_stack(Boolean(a.truthy() || b.truthy()))
    }

    fn add(runtime: &mut Runtime) {
        let b = runtime.pop_value_from_stack();
        let a = runtime.pop_value_from_stack();

        runtime.push_value_to_stack(a.add(&b))
    }

    fn sub(runtime: &mut Runtime) {
        let b = runtime.pop_value_from_stack();
        let a = runtime.pop_value_from_stack();

        runtime.push_value_to_stack(a.subtraction(&b))
    }

    fn multiply(runtime: &mut Runtime) {
        let b = runtime.pop_value_from_stack();
        let a = runtime.pop_value_from_stack();

        runtime.push_value_to_stack(a.multiply(&b))
    }

    fn divide(runtime: &mut Runtime) {
        let b = runtime.pop_value_from_stack();
        let a = runtime.pop_value_from_stack();

        runtime.push_value_to_stack(a.divide(&b))
    }

    fn modulo(runtime: &mut Runtime) {
        let b = runtime.pop_value_from_stack();
        let a = runtime.pop_value_from_stack();

        runtime.push_value_to_stack(a.modulo(&b))
    }
}

pub mod unary {
    use crate::{runtime::Runtime, value::NativeFunction};

    use std::borrow::Cow;

    pub const NEGATE: NativeFunction = NativeFunction {
        name: Cow::Borrowed("#-"),
        arity: 2,
        fun_ptr: negate,
    };

    pub const NOT: NativeFunction = NativeFunction {
        name: Cow::Borrowed("#!"),
        arity: 2,
        fun_ptr: not,
    };

    fn negate(runtime: &mut Runtime) {
        let a = runtime.pop_value_from_stack();

        runtime.push_value_to_stack(a.negate())
    }

    fn not(runtime: &mut Runtime) {
        let a = runtime.pop_value_from_stack();
        runtime.push_value_to_stack(a.negate())
    }
}
