# Mercenary

After writing so many programming languages myself, I decided that I won't write a single line of code for this language: Instead, I've hired others to do it for me. (Each person was given a pizza as a reward for their efforts.)

People are not allowed to modify previous people's code, and they are encouraged to write it as terrible as possible.

## Contributors
The following lovely people contributed to the original project:

- Lexer (Assembly): [ReNegate](https://github.com/RealNeGate)
- AST Generation (C): [Paradoxical](https://github.com/ThePuzzlemaker/)
- Codegen (C++): [RazzBerry](https://github.com/jhburns/)
- Runtime (RUST): [RealKC](https://github.com/RealKC)
- REPL (TBD): TBD
- Bytecode Optimizations (TBD): TBD
- (Maybe?) Preprocessor
- <others?>

## Build Setup

- Install https://www.nasm.us/ , `gcc`, `gcc`, and [cargo](https://doc.rust-lang.org/cargo/getting-started/installation.html)
- `$ cd /src/runtime`
- `$ cargo run -p merc`

## Installation

- `$ cargo install merc --path="src/runtime/merc/"`
- `$ merc <file1> <file2> -- args`
