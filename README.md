# CCompiler
## Feature
- support most features of the C11 standard exclude:
    - preprocessor
    - `_Static_assert `
    - `_Alignas`
    - `_Generic`
    - `_Alignof`
    - `_Atomic`
    - function definition like 
      `int max(a, b) int a, b;`
    - function pointer
    - variable length array
    - function specifier(`inline` and `_Noreturn`)
    - `typedef`
    - nested struct and union
    - `...`
    - cast expression
    
## Get Started
- download the source code from https://github.com/ddxy18/CCompiler.git
- use cmake to build the target and run