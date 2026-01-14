# C++ Best Practices: The Rule of Three/Five/Zero

This section explores a fundamental concept in C++ resource management: the **Rule of Three/Five/Zero**. Understanding this rule is crucial for writing safe and correct C++ code, especially when dealing with classes that manage resources like dynamic memory, file handles, or network connections.

## The Core Idea

When a class manages a resource, the default compiler-generated special member functions (like the copy constructor or destructor) often do the wrong thing. This leads to issues like shallow copies, double-frees, and resource leaks.

The "Rules" are guidelines for how to handle this:

1.  **The Rule of Three:** If you need to explicitly declare any of the following for your class, you almost certainly need to declare all three:
    *   Destructor
    *   Copy Constructor
    *   Copy Assignment Operator

2.  **The Rule of Five:** With the introduction of move semantics in C++11, the rule was extended. If you write any of the original three, you should also consider the two new move-related functions:
    *   Move Constructor
    *   Move Assignment Operator

3.  **The Rule of Zero:** This is often considered the modern ideal. Structure your code so that you don't need to write *any* of these five special member functions yourself. Instead, rely on standard library classes (like `std::vector`, `std::string`, `std::unique_ptr`, `std::shared_ptr`) to manage resources for you. These classes are already written to handle copying, moving, and destruction correctly.

## What We'll Explore

-   **`main.cpp`**: Demonstrates a "bad" class that violates the Rule of Three. When compiled and run, it showcases the double-free bug that occurs from shallow copying resource handles.
-   **`main-correct.cpp`**: Provides a "good" class that correctly implements the **Rule of Three**. This file shows how explicitly defining the destructor, copy constructor, and copy assignment operator leads to correct, safe resource management.
-   Next, we will evolve the class to implement the **Rule of Five** for better efficiency.
-   Finally, we will discuss how the **Rule of Zero** could be applied to simplify the design.
