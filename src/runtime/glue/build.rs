use std::{env, process::Command};

#[cfg(target_os = "linux")]
const NASM_FORMAT: &str = "elf64";
#[cfg(target_vendor = "apple")]
const NASM_FORMAT: &str = "macho64";
#[cfg(target_os = "windows")]
const NASM_FORMAT: &str = "win64";

fn main() {
    do_nasm();
    let out_dir = env::var("OUT_DIR").unwrap();

    cc::Build::new()
        .object(format!("{}/lexer.o", out_dir))
        .compile("merclex");

    cc::Build::new()
        // I'm getting warnings and frankly I do not care
        .extra_warnings(false)
        .flag("-Wno-sign-compare")
        .flag("-Wno-unused-label")
        .flag("-Wno-missing-braces")
        .flag("-Wno-unused-variable")
        .files([
            in_parser("ast-free.c"),
            in_parser("ast-visit.c"),
            in_parser("ast.c"),
            in_parser("parser.c"),
            in_parser("pp.c"),
        ])
        .compile("mercparse");

    cc::Build::new()
        .extra_warnings(false)
        .cpp(true)
        .flag("-std=c++2a")
        // Not my code, not my problem!
        .flag("-Wno-sign-compare")
        .files([
            in_codegen("ast.cpp"),
            in_codegen("instructions.cpp"),
            in_codegen("middle_end.cpp"),
        ])
        .compile("merccodegen");

    println!("cargo:rerun-if-changed=src/codegen-glue.cpp");
    cc::Build::new()
        .cpp(true)
        .flag("-std=c++2a")
        .file("src/codegen-glue.cpp")
        .compile("mercglue")
}

fn in_lexer(file: &'static str) -> String {
    format!("../../lexer/{}", file)
}

fn in_parser(file: &'static str) -> String {
    format!("../../parser/{}", file)
}

fn in_codegen(file: &'static str) -> String {
    format!("../../codegen/{}", file)
}

fn do_nasm() {
    println!("cargo:rerun-if-changed={}", in_lexer("lexer.asm"));

    let out_dir = env::var("OUT_DIR").unwrap();

    let mut cmd = Command::new("nasm");

    cmd.arg(format!("-f{}", NASM_FORMAT));
    #[cfg(debug_assertions)]
    {
        cmd.arg("-g");
    }
    cmd.arg(in_lexer("lexer.asm"))
        .arg(format!("-o {}/lexer.o", out_dir));

    let status = cmd.status().unwrap_or_else(|e| {
        panic!("oops: {}", e);
    });

    if !status.success() {
        panic!("nasm says no");
    }
}
