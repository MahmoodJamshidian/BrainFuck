BrainFuck Programming Language
==============================

BrainFuck is originally a very low-level programming language created by Urban Müller in 1993.

This language is not made for programming and implementing real projects and is only for programmers' entertainment.
This programming language works on memory houses of RAM and (typically) uses only 8 characters **`+`** , **`-`** , **`>`** , **`<`** , **`[`** , **`]`** , **`.`** , **`,`** .

read more from [wikipedia](https://en.wikipedia.org/wiki/Brainfuck).

you can read [Basics of BrainFuck](https://gist.github.com/roachhd/dce54bec8ba55fb17d3a) article to learn its standard version.

I intend to develop this language as much as possible, just for fun and to learn C++ better.

What are the development goals?
-------------------------------

- Rebuilding the compiler
- Pointer System
- Function definition system, storage and use
- Debug System

Which of these goals have been completed?
-----------------------------------------

- ### Rebuilding the compiler
  We know that all programming languages each have their own structure and no programming language is irregular.
  Before I created this repository, I had written a compiler, but it had development problems, because I did not pay attention to the fact that the structures should be defined and then processed. That is, in this way: open the source file, scroll over its characters and if its value is equal to ( **`+`** ), do a certain thing, if it is ( **`-`** ), do a certain thing and...
  
  I encountered many problems in this method, and one of the biggest problems was that it gave a syntax error at runtime and not at compile time, and this is not true at all.

  Suppose you have written a program in C language and you intend to run it. You build it and it runs without any errors, but it encounters a syntax error during execution. Well, this does not make sense at all! If you had a typo, how could the machine read it?
  
  During the build, programming languages are basically read from the source, and according to the source, structures are defined for the compiler, and it is necessary to convert those structures into machine language or to be executed by an interpreter.

  Summary. So far all the main commands and structures have been added to the compiler

  > NOTE: the amount of memory and the number of pointers can be controlled with the -m (amount of memory) and -p (number of pointers) options (which are usually unlimited). When more memory is needed, you will encounter MemoryOverflow error and when more pointers are needed, you will encounter PointerOverflow error.

- ### Pointer System
  One of the problems I faced while writing a small program with this programming language was the problem of moving between memory cells.
  
  When you want to move between memory houses, you use two characters ( **`<`** ) and ( **`>`** ). But consider that you have to write a program that takes a long input from the user, but you can't. If you have doubts, you can try this! If you succeeded, I congratulate you, but take a look at the code you typed, you just wasted your time! But you have to look for a solution.
  
  The idea that came to my mind was to use the pointer system. Each of the pointers can point to a part of the memory and it can be specified where each one points to in the memory.
  
  In this method, four new characters were added to this language:
  - **`/`** : next pointer
  - **`\`** : prev pointer
  - **`{`** : open pointer loop
  - **`}`** : close pointer loop
  
  next pointer ( **`/`** ) and pre pre pointer ( **`\`** ) to select pointer and switch between them.

  > NOTE: when you go to the next pointer and there is no pointer, a new pointer is created, and the new pointer is placed in the position of the previous pointer. This means that the new pointer points to a part of the memory that the previous pointer points to

  **`{`** and **`}`** characters are used for pointer loop
  
  pointer loop is a loop that repeats until the selected pointer reaches the first memory location. Like the normal loop, except that it works on the value inside the memory cells, but the pointer loop works on its position.

  an example of a pointer system
  
  **[gest_passwd.bf](examples/gest_passwd.bf)**
  ```brainfuck
  +++++++[>+++++++<-]>[>+>+>+>+<<<<-]>>+>++>+++
  {<}
  />>
  \++++++[>+++++++++++<-]>+++.
  <++++[>++++++++++<-]>+.
  ++++++.
  <+++[>-----<-]>.
  <+++[>++++<-]>+.
  <++++++++[>----------<-]>--.
  <+++++[>+++++++<-]>.
  <++++[>+++++++++++<-]>.
  -----------.
  +.
  <+++[>--------------<-]>-.
  <+++++[>-----<-]>-.
  [-]
  {<}
  >+<,.[/-\-]/[[-]\>-</]>\
  >+<,.[/-\-]/[[-]\>-</]>\
  >+<,.[/-\-]/[[-]\>-</]>\
  >+<,.[/-\-]/[[-]\>-</]>\
  /<++++[>+++++++++++<-]>++++\>
  [-<+>/+\]
  ++++[<->-]<
  /<\
  [[-]/+\]
  >++++++++++>>++++[<++++++++>-]
  >>>>+<<<<<
  /[
        [-]
      <
      \<.>/
      ++++++++++[>++++++++++++<-]>+.
      ----------.
      ++++++.
      <+++++[>-----------------<-]>.
      <+++++++[>+++++++++++<-]>-.
      +++.
      ++++.
      <++[>-------<-]>.
      <++++++[>-----------<-]>--.\<.>/
      <+++[>+++++++++++<-]>+.
      <++++[>+++++++++++<-]>.
      +++.
      .
      <+++[>----<-]>-.
      --.
      <++++[>++++<-]>+.
      \./
      <++++[>----<-]>-.
      +++++.
      -------.
      <++++[>++++<-]>+.
      <++++[>----<-]>-.
      ++.
      <++++[>++++<-]>+.
      <+++[>-----<-]>.
      <+++[>++++<-]>+.
      +.
      <+++++[>-----------<-]>--. :
      \./
      >.<
      \<.>/
      >>-
  ]
  {<}>>>>>>>
  [
        [-]<<<
      \<.>/
      ++++++++[>+++++++++++<-]>+.
      <++[>-----<-]>.
      ++++++.
      \./
      ++.
      <++[>--------<-]>++.
      +++++.
      <+++++[>---------<-]>.
      \<.>/
      >>>
  ]
  ```
  When you run this code, it will ask you for a code with four characters, and when you enter it, the entered code will be compared with the original code, and if the code is correct, "YOU WIN!" It is printed on the screen and otherwise the value "you lose!" It is printed and then it prints the correct number of characters.

  The code is "1234" defined in [line 1](examples/gest_passwd.bf#L1).

The End
-------