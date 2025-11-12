# Dimension Progamming Language
This is my first attempt at making my dream language. I am taking my favorite features from languages I've tried, and creating my ideal programming language.

## Dimension is a compiled language
Hopefully I will make it compile all the way to machine code like C does but I'm not entirely sure how to do that yet...

## The Type System
One of the core features that makes Dimension unique is its type system. Dimension is a statically typed language, so each piece of data has a type that is predetermined at compile time. Types in Dimension are notated in square brackets:

```Dimension
  x: [i] //x is a variable of type integer
```

Nothing special yet...

There will be a set of *base types* that are predefined in the language: `[i]`, `[u]`,`[f]`,`[c]` are integer, unsigned integer, floating point, and char respectively.

Additionally, the `[type]` type is the data type for types themselves.

### Dimensions
Any type in Dimension can be given a 'dimension' to turn it into a vector. For example, `3[u]` represents the type of a vector with three unsigned integer components. These 'dimensions' can be stacked, so `3*3[f]` represents a three component vector whose components are each a three component vector of floats (basically a 3x3 matrix).

Since Dimension is statically typed, the dimension of a variable is declared with the variable and cannot be changed.

### Structs & Enums
new custom types are created with the `type [...] is <...>` syntax. These custom types can be composed of any number of component types. Component types can optionally be given names. 

```Dimension
  type [vec3] is 3[f];

  type [Sphere] is (position: [vec3], radius: [f]);
```

Custom types can also be defined as enums using the `oneof` keyword. 
A variable with an enum type can be any one of the defined values.

```Dimension
  type [Suit] is oneof (Hearts Spades Diamonds Clubs);

  type [Shape] is oneof (Triangle: 3[vec3], Sphere: [Sphere]);
```

## Variable Declarations

Variables are declared with `{name} : [type]` and are mutable by default. Immutable constant variables can be declared with `::`. *super-constant* variables are declared with `:::`. These are almost like `#define` macros in C in that they are evaluated at compile time. This means that superconstant variables can only be defined in terms of other superconstants and data literals.

```Dimension
   position : 2*[f]; //declare a 2-component float variable named 'position'

  scale : [f] = 1; //declare a float variable named 'scale', and assign its value to be one.

  pi :: [f] = 3.14159; //declare an immutable float variable named 'pi'

  e ::: [f] = 2.71828; //declare a superconst variable named 'e'

  tau ::: [f] = 2 * pi; //<- This is illegal!! Cannot define a superconst in terms of non-superconst variables!
```

## Vector Literals
Assigning a value to a variable can be done by putting data in `(` `)` brackets. Data members can be separated by either spaces or commas.
```Dimension
  point : 2[f] = (5.4, -6.3);

  matrix : 3*3[i] = (1 3 5 2 4 6 7 9 3);
``` 




## Vector Indexing
Vector data types in Dimension can be indexed into using `@xyzw` notation. Similarly to GLSL, these letter indices can be chained together to create new vectors:
```Dimension
 my_vector : 3[i] = (3 7 -1);

 my_vector@x; //3

 my_vector@yz; // (7 -1)  

 my_vector@zyx; //(-1 7 3);

 my_vector@w; //undefined behavior, probably garbage data of some kind

```
These predefined indices only give access to the first four components of vectors, but any components can be retrieved using the `@` function: 
```Dimension
 my_vector : 3[i] = (3 7 -1);

 my_vector@0; //3

 my_vector@(1 2); // <7 -1>  

 my_vector@(2 1 0); //<-1 7 3>;

 my_vector@(3); //undefined behavior, probably garbage data of some kind

```
the `@` syntax can be stacked for variables with multiple dimensions:
```Dimension
 my_matrix : 2*2[i] = ((1 2) (3 4))

 my_matrix@0@1; //2

 my_matrix@(1 0); //((3 4)(1 2))
```

## Functions
Functions in Dimension can take on many forms. Classic C-style functions, operator overloads, and OOP method-style functions can all be defined with the same syntax. Function definitions are notated with  the `fn` keyword. In definitions, parameters are written in parentheses, labeled with types, and separated by commas.

* `fn print_number(n: [i]) ...` 
* `fn (a: [R]) + (b: [R]) ...`
* `fn (B: [Shape]).display() ...`

Functions must also declare the type they return using the `makes` keyword. Functions that do not return anything may omit the `makes` statement.

* `fn print_number(n: [i])` (doesn't need return anything)
* `fn (a: [R]) + (b: [R]) makes [R]` (returns data of type `[R]`)

The body of the function is defined after the `does` keyword. The order of the keywords after `fn` does not matter.

* `fn {...} makes [...] does {...}` 
* `fn {...} does {...} makes [...]`
are both valid ways to declare functions.

Functions can optionally include a `priority` statement. When grouping functions and parameters, the compiler will look at the function's priority level to determine what the order of evaluation should be. This feature is mainly intended for creating custom orders of operations for custom operators. This can lead to some confusing evaluation orders, so programmers should try to use `priority` as little as possible and use parentheses to reduce ambiguity.

* `fn {(a: [i]) + (b: [i])} makes [i] does {a + b} priority 4`

### Lambda Expressions?

I don't know if I want dimension to have lambda functionality, but if it does it will look something like this:

`my_func : [(x: [i], y: [i]) makes [i]] does {...}`

so the type of this variable would be `[([i] [i]) makes [i]]`

### Parameters

Functions can modify the values of parameters that are passed to them by default. To prevent this behavior, parameters must be declared using the constant `::` syntax. Parameters declared with the `:::` syntax are evaluated at compile time and can only be populated with superconstant values.



### Component-Wise Operations
Let's say you make a function to multiply a float by three and add one. 
```Dimension
fn {three_n_plus_one(n : [f])} makes [f] does {3 * n + 1}
```

Dimension will let you call this function on a vector of floats of any size. The function will be called on each component, and the result will be returned in a vector of the same size. For example, `three_n_plus_one((-1, 0, 1))` will return `(-2, 1, 4)`.

For functions with multiple parameters, all parameters must either be single values or vectors with matching numbers of components, otherwise Dimension will throw an error.
## Type Parameters

This feature is similar to Generics in lanugages like TypeScript and Java.

To make code as reusable as possible, functions and type declarations can take parameters that are themselves types or dimensions. For example, let's say we are making a hashMap type, and we want to be able to reuse its code to map between any two data types. These parameters are evaluated at compile time, so they can only be populated by superconstant values. These "super constant" parameters are again indicated with the `:::` syntax. We can declare the type as something like this:

```Dimension
  type [HashMap(t1 ::: [type], t2 ::: [type])] is ...
``` 
Then, when we go to use this HashMap type, we can input the types we need for our specific use case. This example is a map from integers to chars.

```Dimension
  my_map : [HashMap([i], [c])];
``` 

Functions can also have type parameters. For example, this function takes a value of any type, and returns a value of the same type, but with a dimension of 2.

```Dimension
fn {double-ify(V: [t ::: [type]])} makes 2*t does {
  return (V V);
}
``` 
## Dynamic Memory
All data types I've mentioned so far are of a fixed size and allocated on the stack, but the language would not be complete without a way to allocate memory dynamically. This can be done in Dimension using the `+` syntax. There will also be built-in `push()`,`pop()`, and `resize()` functions to modify the sizes of these arrays

```Dimension
my_empty_array : [i]+; //initialized as a dynamic array of ints with zero elements.

my_empty_array.push(5); //the array now contains a single element

my_empty_array.resize(20); //the array now contains 20 elements. The first is still 5, but the rest are uninitialized garbage data.
```
Composite types can also contain dynamic members. Dynamic members allow for creating recursive data structures like trees, graphs, and linked lists.

```Dimension
type [Tree(t ::: [type])] is <value: t, branches: [Tree]+>

type [LinkedList(t ::: [type])] is <value: t, next: [LinkedList]+>
```




## Memory Management
Garbage Collection?
I'm not sure yet...

Here's what I've thought so far:
* **No mark and sweep** - that seems really hard to do well
* **No malloc/free** - that's just a nightmare for coders to manange, but maybe an early version of the language could use it until I find something better
* **Rust-style memory rules?** - It is an elegant solution for sure, but I don't want to be too restrictive on the programmers
* **Reference Counting?** - This intuitively seems like it *should* be the best way to manage memory, but all the info online says that the overhead of storing and incrementing a counter for each reference is huge and not worth it. I find that hard to believe but maybe I could try it out because it seems like it would be much easier to implement.

Whatever system I end up using, I definitely want to design the language so that it encourages minimal dynamic memory allocation.



