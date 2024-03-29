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

- Rebuilding the Interpreter
- Pointer System
- Virtualization System
- Functional System
- Compiler System
- Use Directly
- Use Directly (For Python)
- Plugin Management System

Which of these goals have been completed?
-----------------------------------------

- ### Rebuilding the Interpreter

  We know that all programming languages each have their own structure and no programming language is irregular.
  Before I created this repository, I had written a compiler, but it had development problems, because I did not pay attention to the fact that the structures should be defined and then processed. That is, in this way: open the source file, scroll over its characters and if its value is equal to **`+`**, do a certain thing, if it is **`-`**, do a certain thing and...
  
  I encountered many problems in this method, and one of the biggest problems was that it gave a syntax error at runtime and not at compile time, and this is not true at all.

  Suppose you have written a program in C language and you intend to run it. You build it and it runs without any errors, but it encounters a syntax error during execution. Well, this does not make sense at all! If you had a typo, how could the machine read it?
  
  During the build, programming languages are basically read from the source, and according to the source, structures are defined for the compiler, and it is necessary to convert those structures into machine language or to be executed by an interpreter.

  Summary. So far all the main commands and structures have been added to the interpreter

  > **NOTE**: In the previous versions ([v0.1](https://github.com/MahmoodJamshidian/BrainFuck/releases/tag/v0.1) and [v0.2](https://github.com/MahmoodJamshidian/BrainFuck/releases/tag/v0.2)), there were `-m` and `-p` options that specified the maximum memory limit and the maximum number of pointers, but due to the addition of the function, these options were removed.

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

- ### Functional System
  
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

- ### Compiler System
  
  One of the topics that was very important for us was the ability to provide executive output. If you give the source file to someone and say that it is necessary to install the interpreter and use it to run the program, the client will be curious about what is inside the source and will try to read it. If you build it and give it to the customer, the customer's curiosity will decrease and the process of downloading the interpreter will be removed.
  
  In version [v0.4](https://github.com/MahmoodJamshidian/BrainFuck/releases/tag/v0.4), this possibility was added to this interpreter, and you can run a brainfuck source directly and build it.

  When you compile a program, basically, it turns it into a c++ code where the necessary files are included in the `include` directory and your code is also saved as `c++ structures` and compiles it.

  By this possibility, several switches were added to this compiler:
  - `-b` `--build` : Build executable file
  - `-o <output file>` : The output path of the executable file (source path by default)
  - `-c <c++ compiler>` : Set c++ compiler (g++ by default)
  - `--option <option>` : Set c++ compiler options
  
  > **NOTE**: If you don't use the `-b` or `--build` switch, it will work as an interpreter. And if you have not used these switches and have used the rest of the switches related to the build, you will encounter an error.

- ### Use Directly
  
  Sometimes we want to combine our program with other languages and we don't want to use the compiler to run the brainfuck source separately. If we wanted to do this, we needed to use the source and compiler or the compiled source. We used to execute it by the system, that is, as follows:
  ```cpp
  system("brainfuck ./main.bf");
  // or (after build main.bf)
  system("./main");
  ```

  Maybe we wanted the output not to be printed on the screen and the output to be automatically dumped in the variable??

  With this mentioned method, the work becomes very difficult and it may not work as we want, so we use the [v0.4](https://github.com/MahmoodJamshidian/BrainFuck/releases/tag/v0.4) version that we can include the source compiler into our project and do a lot of work with do it.

  A small example is below:
  ```cpp
  // Example 1
  #include "libbfx.cpp"

  const char *code = "+++++++++[>++++++++++<-]>++++.+.";

  Structure main_struct(code);
  Program program(&main_struct);

  int main()
  {
      program.run();
      return 0;
  }
  ```

  The source of brainfuck is placed in the `code` variable, its structures are specified in the next line, and an instance of the `Program` class is created, and the structure is given to it and finally executed.

  In the line where environment is defined, you may get a little confused, so I will give a brief explanation about this and continue with the content.
  
  The Program class takes other arguments (*which are optional*) that I will explain below.

  The second argument is a pointer to an `iostream` instance. You can control the input and output of your brainfuck program by means of the `iostream` class.

  > **NOTE**: If you don't use `iostream`, the input and output of your brainfuck program will be `STDIO`.

  Consider the following example:
  ```cpp
  // Example 2
  #include "libbfx.cpp"
  #include <iostream>

  const char *code = "++[->(,>,[-<+>]<.)<]";

  iostream stream;
  Structure main_struct(code);
  Program program(&main_struct, &stream);

  int main()
  {
    stream << "\2\43\12\54";
    program.run();
    std::cout << "result is:";
    char loc_c;
    for (int i = 0; i < 2; i++)
    {
      stream >> loc_c;
      std::cout << " " << (int)loc_c;
    }
    std::cout << std::endl;
      
    return 0;
  }
  ```

  We may want to write a program when we output the program according to the input, so we must use signals:
  ```cpp
  // Example 3
  #include "libbfx.cpp"
  #include <iostream>

  const char *code = "++[->(,>,[-<+>]<.)<]";

  char inp[] = "\32\54\12\3";
  int ind = 0;

  iostream stream;
  Structure main_struct(code);
  Program program(&main_struct, &stream,
    [](){
      // input signal handler
      stream << inp[ind++];
    },
    [](){
      // output signal handler
      char loc_c;
      stream >> loc_c;
      std::cout << " " << (int)loc_c;
    }
  );

  int main()
  {
    std::cout << "result is:";
    program.run();
    std::cout << std::endl;

    return 0;
  }
  ```

  The third and fourth arguments of the `Program` class both take a lambda, the third argument is related to the input signal and the fourth argument is related to the output signal.

  To be able to load your desired plugin in this mode, you must use the `load_plugin` function. Pay attention to the following example:
  ```cpp
  // Example 4
  #include "libbfx.cpp"

  const char *code = "+++++++++[>++++++++++<-]>++++.+~.";

  load_plugin("/home/mjscript/projects/BrainFuck/plugins/bin/rev.so", false);

  Structure main_struct(code);
  Program program(&main_struct);

  int main()
  {
    program.run();
    return 0;
  }
  ```

  The first argument of the load_plugin function is the address of the built file of the plugin, and the second argument is a boolean that specifies whether the desired plugin has the access to rewrite the structures or not.

- ### Use Directly (For Python)
  After the purpose of Use Directly was achieved, I got this idea why not combine it with other languages?? That's why I decided to add the possibility of using Python for the next step.

  To be able to do this, I used the Cython language, which made my job very simple.

  After the compilation process is finished, depending on your operating system, it produces either a `libbfx.pyd` or `libbfx.so` output, which you need to place alongside the `libbfx.pyi` file in your project and import it. Below are some examples written in the Python language, as mentioned above.

  ```python
  # Example 1
  import libbfx

  code = b"+++++++++[>++++++++++<-]>++++.+."

  main_struct = libbfx.bfx_Structure(code)
  program = libbfx.bfx_Program(main_struct)

  program.run()
  ```

  ```python
  # Example 2
  import libbfx

  code = b"++[->(,>,[-<+>]<.)<]"

  main_struct = libbfx.bfx_Structure(code)
  stream = libbfx.bfx_iostream()
  program = libbfx.bfx_Program(main_struct, stream)

  stream.write(b"\2\43\12\54")

  program.run()

  print("result is:", end="")

  for i in range(2):
    print('', ord(stream.read(1)), end="")
  print()
  ```

  ```python
  # Example 3
  import libbfx

  code = b"++[->(,>,[-<+>]<.)<]"
  inp = "\32\54\12\3"
  ind = 0

  main_struct = libbfx.bfx_Structure(code)
  stream = libbfx.bfx_iostream()

  def on_read():
    global ind, stream
    res = f"{inp[ind]}".encode()
    ind += 1
    stream.write(res)

  program = libbfx.bfx_Program(main_struct, stream, lambda: on_read(), lambda: print('', ord(stream.read(1)), end=""))

  print("result is:", end="")

  program.run()

  print()
  ```

  ```python
  # Example 4
  import libbfx

  code = b"+++++++++[>++++++++++<-]>++++.+~."

  libbfx.bfx_load_plugin(b"/home/mjscript/BrainFuck/plugins/bin/rev.so", False)

  main_struct = libbfx.bfx_Structure(code)
  program = libbfx.bfx_Program(main_struct)

  program.run()
  ```

- ### Plugin Management System
  BrainFuck itself is a minimalistic language and it is very difficult to write a single line on the console with the available features. BFX is an enhanced version of BrainFuck but still has some shortcomings. Before [v0.6](https://github.com/MahmoodJamshidian/BrainFuck/releases/tag/v0.6), in order to add something new to this language, we had to make changes in the original source, but now we can implement many of these changes without changing the original source.

  To create your own plugin, you need to use the [`rev`](plugins/src/rev.cpp) sample plugin structure as follows:

  ```cpp
  #ifdef BFX_BUILD
  #ifndef BFX_CORE
  #include "core.cpp"
  #endif
  #else
  #include "libbfx.cpp"
  #endif

  Structure S_NAME(
  #ifndef BFX_BUILD
    "NAME", [](size_t __index, const char *__src) -> STR_DATA
    {
      // detector section
      return {__index, __index, &S_NAME};
    }, [](STR_DATA *str) -> std::string
    {
      // builder section
      return "{&S_NAME}";
    }, 
  #endif
    [](Environment *env, STR_DATA *str)
    {
      // reacter section
    }
  );
  ```

  In each plugin package, you can define unlimited structures and note that their names should not be repeated or their names should not be the same as the names of builtin structures (unless you want to rewrite them).

  Another use of plugins is to overwrite builtin plugins, to do this you need to name the new structure the same as the predefined structure. But this possibility is only available in interpretive mode, and when you do the build, the new structure is added to the list of structures, but *it does not replace the previous structure*.

  Next, you need to save it in this format: `plugins/src/PLUGIN_NAME.cpp`, like this: `plugins/src/rev.cpp`.

  To build plugins, refer to the [Build Plugins](#build-plugins) section

  > **Note**: If you have built your plugins arbitrarily, put their source in `plugins/src` and the output in `plugins/bin` to be accessible to the program.

  Next, use the list.txt file to determine which plugins the program should use and which of the plugins can overwrite the predefined structures.

  In each line, write the name of a plugin (name only) and put the + sign at the end so that the plugin can overwrite the previous structures. And to create a comment, put # at the beginning of that line and then your comment.
  for example:

  ```
  # this is a comment
  # The following line causes the rev plugin to be added along with rewriting access to structures.
  
  rev+
  ```

  If the plugin does not have structure rewriting access and a structure is overwritten in it, when the program is executed, you will encounter an error similar to the following:
  ```
  InternalError: error on loading '.../plugins/new_add.so': ADD structure has been rewritten (Permission Denied)
  ```


How To Build
------------
  - ### Prerequisites
    
    To build this project, it is necessary to install `g++` and make on your system. If your operating system is Windows, you can install these two tools by installing `MinGW`, and for Linux operating system, you need to install `make` and `build-essential` packages.

    If you want to build `libbfx`, you need `python` (the version in which you want to use this library) and `cython` and `python-development` (the necessary headers for the build, if you use Linux, it needs to be installed separately and If you are using Windows, it does not need to be installed separately and it will be installed along with the cython installation)

    > **NOTE**: If you want to build the `bfx` compiler or `libbfx` for Windows use git bash as your terminal.

  - ### Build BFX Compiler
    To build the compiler, enter the following command in the terminal:
    ```shell
    make
    ```

  - ### Build Libbfx For Python
    To build `libbfx` for `Python`, you need to use the following command:
    ```shell
    make compile-pyx
    ```

    > **NOTE**: The values of some variables in the `Makefile` file may not be the same as the information in your system. For example, `PYTHON` or `CPPC` variable. You can change those values based on your system information.
  
  - ### Build Plugins
    Use the following command to build plugins:
    ```shell
    make build_plugins
    ```

    With this command, your plugins, which are located in `plugins/src`, will be built and placed in `plugins/bin`.

The End
-------
