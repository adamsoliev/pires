![Pires Logo](./img/pires_compiler.png)

Pires is a C compiler that implements most C99 features and is targeted at RISC-V architecture (RV64IM for now).

## Features roadmap

- Primitive data types: ~~int~~, char, float, double, etc.
- Derived data types: arrays, structures, unions, enumerations, and pointers.
- User-defined data types using typedef.

---

- Conditional statements: if-else, switch-case.
- Looping constructs: for, while, do-while.

---

- ~~Function declaration and definition.~~
- Parameter passing: pass by value, pass by reference.
- Return types: void, ~~int~~, etc.
- Recursive functions.

---

- ~~Variable declaration and initialization~~
- Global vs ~~local~~ vs static variables
- Constant variables (const)
- Automatic variables (auto)

---

- Arithmetic operators: ~~+, -, \*, /~~, %.
- Relational operators: <, >, <=, >=, ==, !=.
- Logical operators: &&, ||, !.
- Bitwise operators: &, |, ^, <<, >>.
- Assignment operators: =, +=, -=, \_=, /=, %=, etc.

---

- Standard input/output functions: printf, scanf, etc.
- File input/output: fopen, fclose, fread, fwrite, etc.

---

- Dynamic memory allocation: malloc, calloc, realloc, free.

---

- Macros: #define, #ifdef, #ifndef, #endif, etc.
- Include files: #include.

---

## Internals

- Tokenize: A tokenizer takes in a string and outputs a list of tokens
- Parse: A recursive descendent parser takes in a list of tokens and constructs ASTs
- Codegen: A code generator emits an RISC-V assembly text for given AST nodes

## Build

```bash
$ git clone https://github.com/adamsoliev/pires
$ cd pires
$ make
```

`Pires` comes with tests. To run them, do:

```bash
$ make test
```

## Example

```bash
$ ./pires "int main() { int a = 23; int b = 32; return a + b; }"
```

Outputs

```
  .globl main
main:
  addi sp, sp, -32
  sd s0, 24(sp)
  addi s0, sp, 32
  li a5, 23
  sw a5, -16(s0)
  li a5, 32
  sw a5, -12(s0)
  ld a5, -12(s0)
  mv a0, a5
  ld a5, -16(s0)
  add a5, a5, a0
  mv a0,a5
  ld s0,24(sp)
  addi sp, sp, 32
  jr ra
```

## References

- [chibicc](https://github.com/rui314/chibicc)
- [tcc](https://bellard.org/tcc)
- [lcc](https://github.com/drh/lcc)
- [c](https://github.com/andrewchambers/c)
- [lacc](https://github.com/larmel/lacc)
- [scc](http://www.simple-cc.org/)
- [acwj](https://github.com/DoctorWkt/acwj)
- [gcc](https://github.com/gcc-mirror/gcc)
- [llvm](https://github.com/llvm/llvm-project)
