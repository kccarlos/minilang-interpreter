# Minilang Interpreter
Compiler coursework (C++) - An interpreter for a programming language called minilang

## To compile

- Environment requirements: Ubuntu 23.04
- C++ version: C++17

```
git clone https://github.com/kccarlos/minilang-interpreter.git
cd minilang-interpreter/
make
```

## To run

```
# To print the tokens (result of lexer)
./minilang -l example.minilang

# To print the AST (result of parser)
./minilang -p example.minilang

# Execute the program
./minilang example.minilang

# Interactive mode
# Use ctrl + d to send EOF signal to exit
./minilang

>> var a;
>> a = 1;
>> println(a);
Result: 1
```

## Minilang syntax

- Variable declaration: `var <variable_name>;`
- Variable assignment: `<variable_name> = <expression>;`
- Function declaration: `function <function_name>(<parameter_list>) { <statement_list> }`. Return the value of the last statement.
- Function call: `<function_name>(<argument_list>);`
- Intrinsics functions
  - `print()`, `println()`, `readint()`, 
  - Array related: `mkarr()`, `len()`, `get()`, `set()`, `push()`, `pop()`
  - String related: `substr()`, `strcat()`, `strlen()`
- Control flow: 
  - `if (<condition>) { <statement_list> } else { <statement_list> }`
  - `if (<condition>) { <statement_list> }`
  - `while (<condition>) { <statement_list> }`
- Operators: `+`, `-`, `*`, `/`, `==`, `!=`, `>`, `<`, `>=`, `<=`, `&&`, `||`, `!`

## Example program

```
var a;
a = 1;
function f(a, b) {
    a + b;
}

var b;
b = f(1, 2);
b;
```

```
var c;
c = mkarr(1, 2, 3);
var d;
d = get(c, 0) + get(c, 1) + get(c, 2);
```

```
s = "hello";
println(s);
```