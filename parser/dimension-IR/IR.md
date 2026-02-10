# Dimension IR

I'm making my own intermediate representation language because I don't want to learn LLVM
we'll see how this goes!! (i have no idea what I might be getting myself into)

## The Plan

Ok so we need a language that has the low level control of assembly, but is platform agnostic. 
Then we need to make assemblers that assemble the IR for each platform we want to support.

Lets start with the language.

At this level, there are no data types. Everything is just a chunk of binary data.

Since different architectures might have different register layouts, calling conventions, etc., I will leave decisions about where specifically to store things to the individual assemblers.

Dimension IR (like LLVM i think) stores values in virtual registers that can be mapped onto real physical registers on the different architectures. Virtual registers are indicated with a `~`, and each have a number to uniquely identify it in its scope `~0`,

These are examples of the different kinds of values that exist in Dimension IR

```Dimension IR
8355059 //a numeric literal (I hope to add hex and binary literals in the future using 0x and 0b)
~1 //the value in the virtual register `~1`
v4 //the local variable located 4 bytes after the base pointer
p2 //the argument located 2 bytes into the parameter data

```

Dimension IR has a small set of instructions that are similar to assembly instructions. Each instruction is annotated by a value in parentheses which indicates the size of the data being operated on.



