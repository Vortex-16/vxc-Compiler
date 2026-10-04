# Vortex Compiler (VXC)
Phase 1: language specification and a working lexer prototype.

## Build
```bash
make
./build/vxc examples/hello.vxc
```

## Current scope
- identifiers and keywords: `fn`, `let`, `int`, `print`
- integer literals
- operators: `+ - * / = : ; ( ) { }`
- whitespace and `//` comments
- line/column diagnostics

The parser, semantic analyzer, IR, optimizer, and VM will be added in later phases.

## Roadmap & Architecture Phases
- [x] **Phase 1: Lexer & Specification** (Tokenization, CLI, LaTeX spec)
- [ ] **Phase 2: Parser & AST** (Recursive descent parser, AST node definitions, AST printer)
- [ ] **Phase 3: Semantic Analysis & Type Checker** (Symbol table, scope checking, type inference/checking)
- [ ] **Phase 4: Intermediate Representation (IR) & Bytecode** (Linear 3-address code or stack bytecode generation)
- [ ] **Phase 5: Virtual Machine / Runtime** (Stack-based execution engine, standard library built-ins)
- [ ] **Phase 6: Basic Optimizations** (Constant folding, dead code elimination)

## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
