use std::{cell::RefCell, fs::File, io::Read, path::Path, process::exit, rc::Rc};

use clap::{crate_authors, crate_version, App, Arg};
use runtime::value::Value;
use tracing::error;

fn main() {
    tracing_subscriber::fmt().init();

    let matches = App::new("The Reference Mercenary Interpreter")
        .version(crate_version!())
        .author(crate_authors!())
        .arg(
            Arg::with_name("INPUT")
                .help("The mercenary file to execute")
                .required(true),
        )
        .arg(
            Arg::with_name("argv")
                .help("Arguments that the `main` of the passed script will receive")
                .allow_hyphen_values(true)
                .takes_value(true)
                .value_delimiter(" ")
                .multiple(true),
        )
        .get_matches();

    let file_path = matches.value_of("INPUT").unwrap();
    let mut file = File::open(file_path).unwrap();
    let mut buf = vec![];
    file.read_to_end(&mut buf).unwrap();

    let argv = match matches.values_of("argv").map(|s| s.collect::<Vec<_>>()) {
        Some(argv) => {
            let mut argv = argv
                .into_iter()
                .map(|s| Value::String(s.into()))
                .collect::<Vec<_>>();
            argv.insert(0, Value::String(file_path.into()));
            Value::List(Rc::new(RefCell::new(argv)))
        }
        None => Value::Null,
    };

    let insns = glue::parse_instructions_from_buf(&buf).unwrap();

    let base_path = match Path::new(file_path).canonicalize() {
        Ok(base_path) => base_path,
        Err(why) => {
            error!("Failed to canonicalize path {:?}, error={}", file_path, why);
            exit(1);
        }
    };
    let base_path = match base_path.parent() {
        Some(base_path) => base_path,
        None => {
            error!("Path {:?} lacks a parent", base_path);
            exit(1);
        }
    }
    .to_path_buf();

    let mut merc_runtime = runtime::runtime::Runtime::create(
        Box::new(|path, base_path| {
            let path = base_path.join(path).canonicalize().unwrap();
            let mut file = File::open(path).unwrap();
            let mut buf = vec![];
            file.read_to_end(&mut buf).unwrap();

            glue::parse_instructions_from_buf(&buf)
        }),
        runtime::intrinsics::INTRINSICS,
        argv,
        base_path,
    );

    merc_runtime.execute_program(&insns);

    let return_value = merc_runtime.pop_value_from_stack();
    drop(merc_runtime);
    drop(file);
    std::process::exit(return_value.to_integer() as i32)
}
