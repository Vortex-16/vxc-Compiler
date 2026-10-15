# Vortex Compiler (VXC)
Vortex Compiler (VXC) is an educational C compiler targeting a custom stack-based virtual machine.

## Prerequisites & Requirements

VXC requires a C11-compliant compiler (`cc`, `gcc`, or `clang`) and `make`.

### Linux (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install build-essential
```

### Linux (Fedora/RHEL)
```bash
sudo dnf groupinstall "Development Tools"
```

### macOS
Install the Xcode Command Line Tools:
```bash
xcode-select --install
```

### Windows
You can build using **Git Bash** with **LLVM-MinGW** or **MSYS2**:
1. Install LLVM-MinGW via winget:
   ```cmd
   winget install MartinStorsjo.LLVM-MinGW.UCRT
   ```
2. In Git Bash, verify `clang` and `make` are available:
   ```bash
   clang --version
   make --version
   ```

## Build & Run

```bash
# Build the compiler
make

# Run the parser on an example program (prints AST by default)
./build/vxc examples/hello.vxc

# Or on Windows Git Bash / Command Prompt:
./build/vxc.exe examples/hello.vxc

# Inspect raw token stream
./build/vxc examples/hello.vxc --dump-tokens

# Run semantic analysis and type checking
./build/vxc examples/hello.vxc --check

# Run test suite
make test
```

## Current scope
- Lexer: identifiers, keywords (`fn`, `let`, `int`, `print`), literals, operators, comments
- Recursive-descent Parser: functions, statements (`let`, `print`, expr), binary expressions with precedence
- AST Pretty Printer (`--dump-ast`)
- Semantic Analysis & Type Checker: scoped symbol table, variable declaration/redeclaration rules, type compatibility, and `main` entry validation (`--check`)
- Line/column diagnostics & panic-mode error recovery
- Automated regression test suite (`make test`)

## Roadmap & Architecture Phases
- [x] **Phase 1: Lexer & Specification** (Tokenization, CLI, LaTeX spec)
- [x] **Phase 2: Parser & AST** (Recursive descent parser, AST node definitions, AST printer)
- [x] **Phase 3: Semantic Analysis & Type Checker** (Symbol table, scope checking, type inference/checking)
- [ ] **Phase 4: Intermediate Representation (IR) & Bytecode** (Linear 3-address code or stack bytecode generation)
- [ ] **Phase 5: Virtual Machine / Runtime** (Stack-based execution engine, standard library built-ins)
- [ ] **Phase 6: Basic Optimizations** (Constant folding, dead code elimination)

## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
