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
