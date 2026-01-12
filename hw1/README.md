## Preliminaries 

Every assignment in this course has two parts: an initial part, which may be completed during a lab session for a small course credit, and a remainder, which you should complete by the due date.

The due date of the assignment is provided on Piazza and in lecture, but is generally before the start of class, one week after the assignment was presented in class. There is no way to submit
a solution for grading - instead you will be evaluated on your understanding of the topics covered
by the assignment during a brief in-class quiz on the due date. As a result, the focus of your effort should not be on "completing" the assignment, but on figuring out and fully understanding what you are doing and why, while completing it. 

Feel free to work in groups, consult AI tools, or any other means at your disposal to better learn the material and prepare for the in-class quiz. Also, feel free to attend more than one lab - because some students choose to not attend labs, there is often
spare capacity for those who need a little extra. 

# Homework 1: Preprocessing, Compiling, Assembling, Linking and Loading

In this assignment, we explore how an executable is built from its components. 
In principle, each component is first individually compiled from ``.c`` code into ``.o``.
The ``.o`` files are then linked to create an executable.

## Lab preliminary step 1: Linux Machine

You will need access to a Linux machine to complete each assignment in this class. The server ``baz.cs.uic.edu`` is available for you to log into, or you may use your local Linux installation if you prefer. 

### Remote server (`baz`) access via SSH (recommended)

On your local machine, install an ssh client. On OS X, this is already available on the terminal. 
On Windows, you might consider using Putty.

* Then, log in to `baz` using the ssh client. Use your regular UIC netid and password.

* Finally, enable passwordless login with ``ssh-keygen`` on your local machine, and ``ssh-copy-id`` to copy the public key to the Linux machine you're using.

### Local install (advanced option)

If you prefer a local Linux installation, you *must not* use "Windows Subsystem for Linux" or any similar compatibility layer: these will invariably lull you into believing they work, and then betray you when you least expect it. 

Instead, use a full virtual machine (VMWare licenses are available for free), dual booting, or simply stick with Linux for everything on your laptop. 
For the least trouble, use Ubuntu 24.04.3 LTS. 

For lab credit, you must *demonstrate:* logging in to a Linux machine that is not running on WSL, either via ssh or locally, without typing in a password.

## Lab preliminary step 2: Prepare a code Editor

You may use any competent code editor for this class. However, most people use one of two editors:
vim, or Visual Studio Code with the Remote SSH extension. Some people use nano. That's not an appropriate choice.

### Using a standard editor instead

If you are among the 99% of people who should just be using a normal editor, install Visual Studio Code on your laptop (it's available for all platforms), and add the Remote-SSH extension. 
* Connect the editor to the ssh server above, and you're able to directly edit server-side files with a user-friendly editor. 

*Demonstrate:* create and save a file on the server using your local editor. 
Then, ssh to the server, and ``cat`` the file, proving that you saved the
file on the server.

### Proving vim proficiency 

Most people use vim because they don't know better. Some people do it despite knowing better. If you choose to use vim and want lab credit, you must prove you are one of them by demonstrating advanced vim proficiency. 

**Demonstrate:** given this file (`README.md`), replace three instances of the string "ing" with "ING" using a single command. That is, you may not use the command three times. Then, quickly navigate to some set of parenthesis and, without entering insert mode or starting a selection, delete the text inside the parenthesis (leaving the empty `()` pair intact).

## Lab step 3: clone the hw1 github repo, build and run your first program

All future steps assume you are working with your preferred editor, and running commands in your Linux environment via ssh. 

* Now use ``git`` to clone the hw1 repository, run ``make static`` in the folder, then ``./static`` to run the first program. 

It should print a few variations of ``Hello World!``. The ``./`` in ``./static`` says "look for the program in the current folder, 
not in the search path ($PATH)". 

*Demonstrate:* If you are using a remote Linux machine, ssh to it without typing in
a password. Then, cd to the hw1 folder and run `./static`.

## Lab step 4: Early adventures in static linking

When you type ``make clean`` then ``make static``, the final ``gcc`` command executed by make creates the file ``static`` from four different ``.o`` files, as specified in the Makefile target. The files ``upper.c`` and ``lower.c`` contain alternative implementations of the function in ``changecase.c``. 

* Changing only the dependencies of the ``static`` target in the Makefile, make it so that the last line of output from ``./static`` shows an uppercase greeting.

*Demonstrate:* Show your updated Makefile target, and the output of `./static`. 

## Lab step 5: Early adventures in dynamic linking

The ``dynamic`` target creates several shared libraries from the same source code, and an executable that depends on ``libchangecase.so``. This dependency is specified using the ``-lchangecase`` flag to gcc: note how the ``lib`` and ``.so`` in the library file name are omitted.

* Run the program using ``./dynamic``. 

You should see an error. The dynamic loader ``ld``
looks for libraries in the library search path ``LD_LIBRARY_PATH``. 

* Override the search path for this one command with ``LD_LIBRARY_PATH=. ./dynamic``. 

You should now see the same output as from ``./static``. 

* Changing only the "recipe" part of the dynamic target, to update the library dependency, make the last line of output have the uppercase version again. 

This "statically" changes the program to depend on a different library. However, 
dynamic library dependencies are actually loaded when the program runs. This means
that we can change the library without recompiling the program, and the program
behavior changes.

* Use ``mv`` to overwrite ``libupper.so`` with ``libchangecase.so``, and run ``./dynamic`` again. 

Now, you should see the original output, despite the program depending on ``libupper.so``! Use ``ldd ./dynamic`` to confirm which libraries ``./dynamic`` will load
when executed. 

*Demonstrate:* run `ldd ./dynamic` to show dependency on ``libupper.so``, then run
the program, showing it still printing the original output. 

## Lab Step 6: LD_PRELOAD

Dynamic symbol resolution provides a number of handy tricks to the systems developer.
With the ``LD_PRELOAD`` environment variable, we can force the dynamic loader ``ld`` to 
load some extra libraries before everything else. If these libraries contain symbols
with the same name as libraries loaded later, these symbols will be used instead. This allows us to override individual functions at runtime. 

* Start from a clean slate with ``make clean`` then ``make dynamic``. Run ``LD_LIBRARY_PATH=. ./dynamic`` to see the original output. 

* Now, override the behavior of the ``changecase`` function by preloading ``libupper.so``. ``LD_LIBRARY_PATH=. LD_PRELOAD=libupper.so ./dynamic``

## Remaining step 7: studying Makefiles 

When you run ``make static`` in the hw1 folder, several files are automatically created. 

Read the Makefile to see how it works. The ``static`` target has three dependencies listed after the ``:``. For files that already exist, such as ``uppercase.c``, no further work is required. However, the file ``main.o`` is automatically built 
from the ``main.s`` file, which is built from ``main.pre.c`` and so on. The
lines below each target (called the "recipe") say how to build the target from the dependencies.

``make clean`` has no dependencies, and since no file named ``clean`` exists, it
always runs, deleting a bunch of previously produced files. 
Run ``make clean`` a couple of times. Then try creating a file called ``clean`` with the command ``touch clean``. Run ``make clean`` again, and observe how the behavior changes. Finally, delete the file with ``rm clean`` to restore normal behavior. 

One key but easily overlooked detail in Makefile syntax is the tab preceding each
line of the recipe below a target. Without the tab, it won't work. Spaces are not ok!

Try running ``make clean`` then run ``ls`` to see which files are still in the folder.
Now, run ``make main.o``, and ``ls`` again. What new files were added? Now run ``make static`` - notice how only one ``gcc`` command was run this time. Why?

Finally, the command ``touch`` updates the modification date of a file. Run ``touch main.c`` to pretend we edited ``main.c``. Now run ``make static`` again. It runs
several ``gcc`` commands this time. What's different? How does ``make`` know
which commands it needs to re-run?

There are several instances of unusual syntax in the Makefile: `$^`, `$@`, `$<` and `${CFLAGS}`. Make sure you understand every detail of this example Makefile in its entirety. Ask a TA, your instructor, or your favorite AI for help if you need help.

Something else slightly unusual about Makefiles presents itself when we look at how a file like `uppercase.o` was built. There are no rules which directly target `uppercase.o`!
However, you will find a rule with the funky syntax `%.o: %.c`. We definitely have no file called `%.o` that needs to be built from a file called `%.c`, and indeed, the `%` Makefile
syntax here is a *wildcard*: this rule will build any otherwise-unmentioned `<something>.c` files into the files `<something>.o`.

What happens if we remove that `%.o: %.c` rule entirely? It still builds just fine, but the output of `make` has changed! What's going on?
**TIP**: Look into the `CC` and `CFLAGS` variables.

## Remaining step 8: from C to executable, step by step

The provided Makefile is designed to produce all the intermediate files from C to executable. Have a look at each intermediate step:

* main.pre.c: the result of running the C processor on main.c. What differences do you notice vs. main.c? Which specific line from main.c is missing in main.pre.c, and what took its place?

* main.s: the result of compiling main.pre.c into x86 assembly. Note the label "greeting" just before the "Hello world" string, and several other greeting-related annotations. How did it get that label?
* Which 3 lines of assembly corresponds to C line ``changecase(greeting);``? Note that the register ``%rdi`` is used to hold the first argument to a function in the AMD64 ABI used here.

* main.o: the result of assembling main.s into a relocatable object file. This file
contains the machine code corresponding to main.c, but with the addresses of various
symbols, such as ``uppercase`` not yet determined. 
* use ``objdump -d main.o`` to disassemble the machine code instructions in main.o. 
Can you spot where those same three lines of assembly are now? 

With the symbol names
obscured, it's harder to spot them. Notice how in the disassembly, every ``lea`` loads ``0x0(%rip)``. In the machine code (the hexadecimals to the left), this is revealed as simply ``00 00 00 00``: an empty placeholder.  

* use ``readelf -a main.o`` to get a different view of the contents of main.o
In the .rela.text section near the end, you'll find the offset of every such placeholder, and the name of the symbol whose address should go there once we've found it. Look for ``changecase``, look up the offset, then have a second look at the ``objdump`` disassembly to confirm that you had the correct lines of assembly. 

* static: the result of linking: combining several .o files, including main.o, into a single executable. At this point, every previously unknown symbol that main uses, such as ``uppercase`` has been found, and every reference to every such symbol is replaced by an actual address. 

* use ``objdump -d static``, to see what the assembly for ``main`` looks like after 
linking. 
* find the instructions that correspond to ``changecase()`` in the main function of static. 
* use ``readelf -a static`` to inspect the executable. It no longer contains a ``rela.text`` section, as those relocation entries have all been resolved. However, it does
contain some relocation entries related to shared libraries, see below. 
* try ``strip static`` then ``readelf -a static`` again. What changed?

## Advanced step 9: fun with shared library relocation

As you noticed in steps 5 and 6, shared libraries can be replaced at runtime. In fact, when the program is linked, we don't even know how many libraries will be loaded, or where! This means the linker can't put the address of a dynamically linked function in the code, the way it can do with a statically linked function.

* use ``objdump -d dynamic`` to observe the difference between the call to the dynamically linked ``changecase``, and the statically linked ``lowercase``. 
The ``lowercase`` call goes straight to 1262, the address of ``lowercase``. 
But where does the ``changecase`` call go? It also has a fixed address...

See how far you can track this by reading the output of `objdump -D dynamic`. FWIW, `0x3fb8` is populated at runtime with an array of symbol name pointers. `0x3fc0` is populated at runtime with the address of a name resolution helper function in `ld-linux-x86-64.so.2`. 

## Advanced step 10: a first look at Rust 

Install rust following the instructions at https://rust-lang.org/learn/get-started/

Then, create a rust project with `cargo new hw1rust`. `cd hw` then `cargo run` to 
build and run the code in `src/main.rs`. It prints a familiar message.

The produced binary is `target/debug/hw1rust`. It is similar in many ways to the C executable, but considerably larger: by default, Rust executables include a lot of extra baggage. This makes using `objdump -D` cumbersome. However, try `objdump --disassemble=main target/debug/hw1rust` to see the produced code for `main`. 

This is actually a wrapper function, not the one in `src/main.rs`. However, you should be able to find the symbol for the actual main function by carefully reading the `objdump` output. See if you can track down the 
actual `call` instruction that prints the string. To make sure you found it, consider changing `src/main.rs` 
to print another string, and see if the same instruction appears twice. 
