BrainFuck Programming Language
==============================

BrainFuck is originally a very low-level programming language created by Urban Müller in 1993.

This language is not made for programming and implementing real projects and is only for programmers entertainment.
This programming language works on memory houses of RAM and (typically) uses only 8 characters **`+`** , **`-`** , **`>`** , **`<`** , **`[`** , **`]`** , **`.`** , **`,`** .

read more from [wikipedia](https://en.wikipedia.org/wiki/Brainfuck).

you can read [Basics of BrainFuck](https://gist.github.com/roachhd/dce54bec8ba55fb17d3a) article to learn its standard version.

I intend to develop this language as much as possible, just for fun and to learn C++ better.

What are the development goals?
-------------------------------

- Rebuilding the compiler
- Pointer System
- Virtualization System
- Functional System
- Debug System

Which of these goals have been completed?
-----------------------------------------

- ### Rebuilding the compiler
  We know that all programming languages each have their own structure and no programming language is irregular.
  Before I created this repository, I had written a compiler, but it had development problems, because I did not pay attention to the fact that the structures should be defined and then processed. That is, in this way: open the source file, scroll over its characters and if its value is equal to **`+`**, do a certain thing, if it is **`-`**, do a certain thing and...
  
  I encountered many problems in this method, and one of the biggest problems was that it gave a syntax error at runtime and not at compile time, and this is not true at all.

  Suppose you have written a program in C language and you intend to run it. You build it and it runs without any errors, but it encounters a syntax error during execution. Well, this does not make sense at all! If you had a typo, how could the machine read it?
  
  During the build, programming languages are basically read from the source, and according to the source, structures are defined for the compiler, and it is necessary to convert those structures into machine language or to be executed by an interpreter.

  Summary. So far all the main commands and structures have been added to the compiler

  > **NOTE**: In the previous versions ([v0.1](https://github.com/MahmoodJamshidian/BrainFuck/releases/tag/v0.1) and [v0.2](https://github.com/MahmoodJamshidian/BrainFuck/releases/tag/v0.2)), there were -m and -p options that specified the maximum memory limit and the maximum number of pointers, but due to the addition of the function, these options were removed.

- ### Pointer System
  One of the problems I faced while writing a small program with this programming language was the problem of moving between memory cells.
  
  When you want to move between memory houses, you use two characters **`<`** and **`>`**. But consider that you have to write a program that takes a long input from the user, but you can't. If you have doubts, you can try this! If you succeeded, I congratulate you, but take a look at the code you typed, you just wasted your time! But you have to look for a solution.
  
  The idea that came to my mind was to use the pointer system. Each of the pointers can point to a part of the memory and it can be specified where each one points to in the memory.
  
  In this method, four new characters were added to this language:
  - **`/`** : next pointer
  - **`\`** : prev pointer
  - **`{`** : open pointer loop
  - **`}`** : close pointer loop
  
  next pointer **`/`** and pre pre pointer **`\`** to select pointer and switch between them.

  > **NOTE**: when you go to the next pointer and there is no pointer, a new pointer is created, and the new pointer is placed in the position of the previous pointer. This means that the new pointer points to a part of the memory that the previous pointer points to

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

- ### Virtualization System (PART mechanism)

  One of the problems that made brainfuck programs complicated and difficult was their relocation, for example, pay attention to this small example:
  We want to store the data in the bytes indexed 0, 1, 2, 3 with the value of 65, but that's not all! The maximum space we have is 4 bytes!
  We can do this in the usual way:

  ```brainfuck
  ++++++[>+++++++++++<-]>-
  [<+>->+>+<<]
  ++++++++++++++++++++++
  +++++++++++++++++++++
  +++++++++++++++++++++
  ```

  This program does this, but in the last part, the number of + characters is too much! Why should the code be like this?
  Well, this is the bad mode we mentioned above. Next, I will explain how this program works:
  In this program, the value of 6 is stored in byte index 0, then it is looped and every time it is subtracted from it and 11 is added to the next byte value, finally the value of byte index 1 is equal to 66 and one is subtracted from it. which becomes 65.

  so the memory will look like this:

  | index | 0 | 1 | 2 | 3 |
  |-------|---|---|---|---|
  | value | 0 | 65| 0 | 0 |

  Next, the value of the index 1 is circled and subtracted from it and added to the indices 0, 2 and 3, and it looks like this:

  | index | 0 | 1 | 2 | 3 |
  |-------|---|---|---|---|
  | value | 65| 0 | 65| 65|

  Only index 1 is left to change its value to 65, but how can do it? If we want to copy the value of another one of these indices, its value will be zero and this loop will continue, so we have to manually adjust its value and finally the value of all memory values will be 65 *(this happens for all amounts of memory)*.

  In this method, three new characters were added to this language:
  - **`&`** : return value
  - **`(`** : open part
  - **`)`** : close part
  
  To use this option, you must put the code you want to simulate inside the open parenthesis and the closed parenthesis and use the **`&`** character to place the value of that part on the main memory.

  > **NOTE**: you can use them nested and you cannot change the movement of pointers and selection pointer inside PART

  this is a rewritten program above using PART:

  ```brainfuck
  ++++++[>+++++++++++<-]>-
  ([<+&>->+&>+&<<])
  ```

  We were able to easily solve this problem.
  
  By using this option, you can better control your memory and solve many complex problems more easily

- ### Function definition system
  
  In all programming languages, repeating a piece of code is a normal thing, and to prevent this from happening, functions are used. But in BrainFuck, it's not just repetition, because if you copy a piece of code from another place and paste it in another place, your program may have problems, because the memory is managed directly. So therefore we need function structure in BrainFuck.

  By this structure, two new characters were added to this language:
  - **`%`** : begin and end of function definition
  - **`$`** : begin and end of function call
  
  You can define function like this:

  ```brainfuck
  %your code here%
  ```

  When a function is defined, the ID of that function is stored in that part of the memory, and in order to execute the function, the value of the memory on that pointer must be equal to the ID of the function, otherwise you will encounter an Undefined error.

  > **NOTE**: the ID of first function definition is 0 and second is 1 and ...

  To call and pass arguments to the function:

  ```brainfuck
  $arg1&arg2&$
  ```

  > **NOTE**: all of operations inside of function call structure is virtual

  > **NOTE**: The `&` character is used in the structure of the function call as the determinant of the function arguments and in the structure of the function definition as the determinant of return values.

  Address the values instead of arg1 and arg2

  > **NOTE**: You can pass unlimited arguments

  When a function is called, its arguments are placed in the function memory like this:

  | index | 0                   | 1    | 2    | ...    |
  |-------|---------------------|------|------|--------|
  | value | lenght of arguments | arg1 | arg2 | ...    |

  You may not have understood correctly, so pay attention to the following example:

  Write a program that adds two numbers 26 and 47 together using the function and stores it in index 0.

  ```brainfuck
  %[-/>\]/{[-\+/]<}&%
  $[-]+++++[->+++++<]>+&<+++[->+++++++<]>&$
  >[-<+>]
  ```

  In the first line, we defined a function that creates a new pointer and moves forward by the number of arguments (index 0), then enters the pointer loop, which drops all the values at index 0, and finally the value at index 0 is returned.

  > **NOTE**: also, function can return any number of values

  In the second line, we called the function and passed 26 and 47 values (If you noticed, in the first loop of the line, we set the value of index 0 to 0, it is true that this is the first function that we have defined and its ID is 0, but it is possible that another function is defined)

  > **NOTE**: After calling the function, the return values will be placed after the pointer point in order

  Well, now the value 73 is stored in index 1, and in the third line, this value is moved to index 0.

  result is:

  | index | 0 | 1 |
  |-------|---|---|
  | value | 73| 0 |

The End
-------