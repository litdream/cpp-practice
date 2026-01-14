# The Pitfall of `std::move`: Use-After-Move

This subproject demonstrates one of the most common and dangerous errors when using `std::move`: accessing an object after its resources have been moved away.


SUMMARY:
Rust ownership model checks this in compile time.
But, C++ only apply std::move, but don't check the original reference is still used.


## What is `std::move`?

`std::move` does not, by itself, move anything. It is a cast that unconditionally converts its argument into an *r-value reference*. This tells the compiler that the object is now a candidate to be moved from.

When the compiler sees an r-value reference being used to initialize or assign to another object, it will prefer to use the class's **move constructor** or **move assignment operator** instead of the copy versions.

A move constructor's job is to "steal" the resources (like heap-allocated memory) from the source object and leave the source object in a "valid but unspecified" state. For pointers, this typically means setting the source pointer to `nullptr`.

## The Problem: Use-After-Move

The most dangerous pitfall is trying to use the moved-from object as if it were still valid. After the move, the object's internal state has been transferred. While the object itself is still in scope and can be destructed, its data is gone.

Attempting to access the moved resources will, at best, show that the object is empty. At worst, it will lead to **undefined behavior**, which often results in a **segmentation fault** (a crash).

### The Example in `main.cpp`

The code in this project demonstrates this exact scenario:

1.  A `Message` class is defined, which holds a `std::string` on the heap via a raw pointer (`payload_`).
2.  The move constructor for `Message` transfers the `payload_` pointer to the new object and sets the original object's `payload_` to `nullptr`.
3.  In `main`, we create a `Message` called `original`.
4.  We then use `std::move` to construct a new `Message` called `destination`. `original`'s payload is moved.
5.  We then call the `getFirstChar()` method on the `original` (moved-from) object.
6.  This method tries to dereference the now-`nullptr` `payload_`, which will cause the program to crash.

## C++ vs. Rust: A Key Difference in Move Semantics

This "use-after-move" pitfall highlights a fundamental difference between C++ and languages like Rust concerning move semantics and compile-time safety.

| Feature                | C++ (`std::move`)                                                                  | Rust (Move Semantics)                                                              |
| :--------------------- | :--------------------------------------------------------------------------------- | :--------------------------------------------------------------------------------- |
| **Concept**            | A resource-stealing optimization implemented by special member functions.          | A fundamental transfer of ownership.                                               |
| **Compiler Check**     | **No compile-time check** for use-after-move. The compiler trusts the programmer.  | **Strict compile-time check** via the borrow checker, preventing use-after-move.   |
| **Moved-From State**   | "Valid but unspecified" state. Object is destructible and assignable, but its data is gone. | Object is considered **uninitialized** and cannot be used after a move.            |
| **Safety Guarantee**   | Programmer is responsible for ensuring correctness; runtime errors are possible.   | Memory safety (including use-after-move) is guaranteed by the compiler.            |

In C++, `std::move` is merely a cast; the compiler does not track the validity of the moved-from object. You can still compile code that attempts to use `original` after its resources have been moved, leading to runtime undefined behavior. In contrast, Rust's borrow checker would prevent such code from compiling, enforcing safety at the earliest possible stage.

## How to Compile and Run

This is a self-contained subproject. From within the `std-move-pitfalls` directory:

1.  **Create a build directory:**
    ```bash
    mkdir build
    ```

2.  **Navigate into the build directory:**
    ```bash
    cd build
    ```

3.  **Run CMake to configure the project:**
    ```bash
    cmake ..
    ```

4.  **Compile the code:**
    ```bash
    make
    ```

5.  **Run the executable and observe the crash:**
    ```bash
    ./std_move_pitfall_example
    ```

You will see output showing the construction and the move, followed by a segmentation fault when the program tries to access the moved-from object. This demonstrates the danger of using an object after its resources have been moved.
