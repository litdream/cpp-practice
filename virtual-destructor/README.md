# C++ Best Practices: Polymorphism and Virtual Destructors

This section explores a critical C++ feature for correct resource management in object-oriented programming: the **virtual destructor**.

## The Core Problem

In C++, it's common to use a base class pointer to manage a derived class object. This is a cornerstone of polymorphism. However, a major problem arises when you `delete` an object through a base class pointer that does not have a `virtual` destructor. Even with modern C++ and smart pointers like `std::unique_ptr`, this rule remains critical because smart pointers ultimately call `delete` on the raw pointer they manage.

```cpp
std::unique_ptr<Base> b = std::make_unique<Derived>();
// ...
// 'b' goes out of scope, unique_ptr calls delete b.get(); // Potential problem is here!
```

If `Base`'s destructor is *not* `virtual`, the compiler only knows to call the `Base` destructor when the smart pointer destroys the object. It is completely unaware that the object is actually of type `Derived`. Any resources allocated in `Derived` (and its destructor) will be leaked, and only the `Base` part of the object will be properly destroyed. This leads to subtle, undefined behavior and resource leaks.

**The Rule:** Any time a class is intended to be used as a polymorphic base class (i.e., you might destroy a derived class object through a pointer to it, even a smart pointer), its destructor **must** be declared `virtual`.

## What We'll Explore

-   **`main.cpp`**: Demonstrates the "bad" scenario where a loop rapidly allocates objects, each consuming ~11MB of memory. Because the base class lacks a `virtual` destructor, this memory is never freed, leading to a significant and easily observable memory leak.
-   **`main-correct.cpp`**: Shows the "good" scenario, allocating and deallocating the same large objects in a loop. With a `virtual` destructor in the base class, resources are correctly freed each time, and the program's memory usage remains stable.

## How to Compile and Run

First, navigate to the `virtual-destructor` directory.

### Demonstrating the Leak (`main.cpp`)

You can compile and run the "bad" example, which will leak memory:
```bash
g++ main.cpp -o main_leak && ./main_leak
```

**To see the leak hit a ceiling**, run it within a shell that has a constrained virtual memory limit (e.g., ~500MB). The program should be killed by the OS.
```bash
(ulimit -v 500000; ./main_leak)
```

### Demonstrating the Fix (`main-correct.cpp`)

Compile and run the "good" example. It will complete successfully with stable memory usage.
```bash
g++ main-correct.cpp -o main_correct && ./main_correct
```



