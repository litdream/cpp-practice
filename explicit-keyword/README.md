# C++ `explicit` Keyword for Constructors

This subproject explains and demonstrates the use of the `explicit` keyword for constructors in C++.

Related link:
- https://www.learncpp.com/cpp-tutorial/converting-constructors-and-the-explicit-keyword/
- https://en.cppreference.com/w/cpp/language/implicit_conversion.html
- https://en.cppreference.com/w/cpp/language/explicit.html



## The Problem: Implicit Conversions

In C++, a constructor that can be called with a single argument automatically defines an **implicit conversion** from the argument's type to the class type. While this can sometimes be convenient, it often leads to unintentional and surprising behavior, which can be a source of subtle bugs.

Consider a class `MyString` that has a constructor taking an integer, meant to reserve a certain capacity:

```cpp
class MyString {
public:
    // Constructor to pre-allocate memory
    MyString(int initial_capacity);

    // ... other methods ...
};
```

Without the `explicit` keyword, the compiler is allowed to use this constructor to perform an implicit conversion from `int` to `MyString`.

### What's the impact of not using `explicit`?

Let's see the potential problems with an example. Suppose you have a function that takes a `MyString` object:

```cpp
void printString(const MyString& s) {
    // ... implementation ...
}
```

Because of the implicit conversion, you could call this function in a way you might not expect:

```cpp
printString(10); // Compiles!
```

The compiler sees that `printString` expects a `MyString`, but it was given an `int`. It then looks for a way to convert `10` into a `MyString`. It finds the `MyString(int)` constructor and uses it automatically. This creates a temporary `MyString` object with a capacity of 10 and passes it to the function.

This might seem harmless, but it can lead to confusing code and hide bugs. A reader of the code might wonder, "What does it mean to print the integer 10 as a string in this context?" The programmer's intent is unclear. Was this a typo? Should they have passed a string literal? The implicit conversion obscures the meaning and can lead to logical errors that are hard to track down.

Another example:

```cpp
MyString s = 'c'; // Compiles! 'c' is promoted to int.
```

Here, the character `'c'` is promoted to its ASCII integer value, and a `MyString` is constructed. This is almost certainly not what the developer intended.

## The Solution: The `explicit` Keyword

To prevent these kinds of unintended implicit conversions, you can mark the constructor with the `explicit` keyword.

**Rule of Thumb:** You should make all single-argument constructors `explicit` unless you have a very good, specific reason to allow implicit conversions.

### How does `explicit` change the behavior?

If we modify our `MyString` class:

```cpp
class MyString {
public:
    explicit MyString(int initial_capacity);

    // ... other methods ...
};
```

Now, the compiler is no longer allowed to use this constructor for implicit conversions. Any attempt to do so will result in a compile-time error.

```cpp
printString(10);      // ERROR: Cannot convert int to MyString
MyString s = 'c';     // ERROR: Cannot convert char to MyString
```

This is much safer. The compiler now forces the programmer to be explicit about their intentions. If they truly want to create a `MyString` from an integer, they must do so explicitly:

```cpp
printString(MyString(10)); // OK: Explicit construction
MyString s(10);            // OK: Direct initialization
MyString s2 = MyString(10);  // OK: Explicit construction, then copy
```

This makes the code clearer, safer, and easier to understand. It prevents the compiler from making assumptions about the programmer's intent.

## How to Compile and Run

This is a self-contained subproject. From within the `explicit-keyword` directory:

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

5.  **Run the executable:**
    ```bash
    ./explicit_keyword_example
    ```
