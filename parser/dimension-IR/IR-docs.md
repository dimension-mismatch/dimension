# Dimension IR Documentation
## Contents
- [About Dimension IR](#about)
  * [Data Types](#data-types)
  * [Registers](#registers)
  * [Instructions](#instructions)
- [Complete Instruction List](#instruction-set)
  * [Integer Arithmetic](#1-integer-arithmetic)
  * [Floating Point Arithmetic](#2-floating-point-arithmetic)
  * [Bitwise Logic](#3-logic-operations)
  * [Comparisons]()
- [Example Programs](#example-programs)

## About

Dimension compiles all its programs into a language called Dimension IR, which stands for Intermediate Representation. Dimension IR is a string of simple instructions that are very easy for computers to execute. Unlike Assembly languages, Dimension IR is not designed for a particular computer architecture or operating system.  As the developer of Dimension, this is nice because it lets me write a single optimizer that works for all dimension code running on all platforms. 

To support Dimension on a new architecture, I will not need to write a whole new Dimension compiler. I can simply write an assembler that translates IR into platform-specific binary instructions. Since Dimension IR is so low-level, it is a nearly one-to-one mapping between IR instructions and the raw binary instructions.


Dimension IR can be written by hand in its own `.dmsnir` files (although I wouuldn't recommend it if you want to keep your sanity).
It is much more useful when written inline in Dimension programs.

### Data Types
One of Dimension's defining features is its super sophisticated type system, but down here at the assembly level, Dimension IR has no concept of types. For example, take the binary string ` 00000000011111110111110011110011`

If you know binary, you might be able to decipher that this number is the integer $8355059$.

However, it would be equally valid to say that this number is the 32-bit float $1.170793134604324179177495 \times 10^{-38}$, or that it is an 8-bit rgba color: $rgb(0, 127, 124, 243)$

In Dimension IR, the only thing that determines the type of this data is what you do with it. If you write it to the screen, it's a color. If you call a floating poing operation on it, it's a float, and if you dereference it, it's a pointer. 

Dimension IR does *not* have a type checker. It has *no idea* what the data it works with actually represents. It is up to the programmer (or compiler) to do the right thing with the right data.

I do plan to add some memory safety features to prevent some segfaults, memory leaks, etc., but for the most part, if you want type safety, use Dimension, and let the compiler handle the IR.

### Registers


When writing Dimension-IR, it is useful to store results of your computations somewhere, but since dimension-IR is platform agnostic, it doesn't know exactly how your computer likes to store its data. To solve this problem, Dimension IR uses a system of "virtual registers". The assembler decides how these map to the real physical registers on the cpu as it depends on the specifics of the target architecture. 

Virtual registers each have a number, and there is (almost) no limit on how many you can have. They are written in code like this: `~67` 
The `~` indicates that this is a register and the `67` indicates that it is register number 67.
### Instructions
Most instructions have two inputs and one output. To store the output of an instruction in a register, 


## Instruction Set
Below is a list of every instruction in Dimension IR.
### 1. Integer Arithmetic
| Instruction | Name | Arguments | Returns | Notes
|:---:|---|:---|---|---|
| `+`  | Integer Addition | `A`,`B` : Integers, signed or unsigned | `A - B`
| `-`  | Integer Subtraction | `A`,`B` : Integers, signed or unsigned | `A + B`
| `i*` | Signed Integer Multiplication| `A`,`B` : Signed integers | `A * B`
| `u*` | Unsigned Integer Multiplication | `A`,`B` : Unsigned integers | `A * B`
| `i/` | Signed Integer Division |`A`,`B` :  Signed integers | `A / B`
| `u/` | Unsigned Integer Division |`A`,`B` :  Unsigned integers | `A / B`

### 2. Floating Point Arithmetic

| Instruction | Name | Arguments | Returns | Notes
|:---:|---|:---|---|---|
| `f+`  | Float Addition | `u/ (size) [A] [B]`
| `f-`  | Float Subtraction |
| `f*` | Float Multiplication
| `f/` | Float Division

*The size argument of floating point instructions must be either `4` or `8` (float or double)*

### 3. Bitwise Logic 
| Instruction | Name | Arguments | Returns | Notes
|:---:|---|:---|---|---|
| `&`  | Bitwise AND | `A`, `B` |  All bit positions where both `A` and `B` have a `1` will be `1`. All others will be `0`
| `\|`  | Bitwise OR |  `A`, `B` |All bit positions where either `A` or `B` have `1` will be `1`. All others will be `0`|  
| `^` | Bitwise XOR | `A`, `B` | All bit positions where either `A` or `B`, *but not both*, have `1` will be `1`. All others will be `0`
| `!` | Bitwise NOT | `V` | The opposite of every bit in `V` - replace `1`'s with `0`'s and `0`s with `1`s
| `<<` | Left Bit Shift | `V` : Any, `n` : Unsigned integer |  Move all bits in `V` to the left `n` spaces. Fill in the right with zeroes.
| `>>` | Right Bit Shift | `V` : Any, `n` : Unsigned integer |  Move all bits in `V` to the right `n` spaces. Fill in the left with zeroes.

### 4. Comparisons
| Instruction | Name | Arguments | Returns | Notes
|:---:|---|:---|---|---|
| `==`  | Equals | `A`, `B` : Any |  returns `true` if `A`'s and `B`'s binary are identical
| `i>`  | Integer Greater Than|  `A`, `B` : Signed Integers| `true` if `A > B`|  
| `i>=` | Integer Greater or Equal| `A`, `B` : Signed Integers | `true` if either `A > B` or `A == B`
| `u>` | Left Bit Shift | `V` : Any, `n` : Unsigned integer |  Move all bits in `V` to the left `n` spaces. Fill in the right with zeroes.
| `u>=` | Right Bit Shift | `V` : Any, `n` : Unsigned integer |  Move all bits in `V` to the right `n` spaces. Fill in the left with zeroes.
```Dimension IR
//data manipulation
move <source> <destination>
deref <pointer>

//bitwise logic
&
|
^
!

//comparisons
==
f>
i>
u>

f<
i<
u<

//system calls (incomplete, more to come)
print <pointer> <length>


```

## Example Programs

### Fibonacci Numbers
This program computes the Fibonacci Sequence in an infinite loop
``` Dimension IR
//store 2 64-bit unsigned integers on the stack
move (8) 0 v0       // put a 0 in the first integer
move (8) 1 v8       //put a 1 in the second integer

.loop-start         //this label marks the start of the loop
~0 + (8) v0 v8      //Add the two variables, and store the result in register ~0
move (8) v8 v0      //set the first variable to the second one
move (8) ~0 v8      //move register ~0 (the sum) into the second variable
jump .loop-start    //jump back to the start of the loop, repeating indefinitely

```