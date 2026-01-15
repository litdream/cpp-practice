#include <iostream>
#include <memory> // For std::unique_ptr
#include <vector> // For std::vector

// --- Base Class ---
// This class does NOT have a virtual destructor, which is the root of the problem.
class Base {
public:
    Base() {
        // std::cout << "Base constructor called.\n";
    }
    // Non-virtual destructor
    ~Base() {
        // std::cout << "Base destructor called.\n";
    }
};

// --- Derived Class ---
// Manages a resource (a dynamically allocated vector of chars).
class Derived : public Base {
public:
    static constexpr size_t RESOURCE_SIZE = 11 * 1024 * 1024; // ~11MB per object

    Derived() {
        // std::cout << "Derived constructor called.\n";
        // Allocate a significant resource
        resource_ = std::vector<char>(RESOURCE_SIZE, 'A'); 
    }
    ~Derived() {
        // std::cout << "Derived destructor called. Resource (11MB) should be freed.\n";
        // std::vector's destructor handles memory freeing, but it won't be called polymorphically.
    }
private:
    std::vector<char> resource_;
};

// Function to demonstrate the issue using unique_ptr within a scope
void createAndDestroyObject(int iteration) {
    // std::cout << "\n--- Entering createAndDestroyObject() scope (Iteration " << iteration << ") ---\n";
    // std::cout << "--- Creating Derived object via Base unique_ptr ---\n";
    std::unique_ptr<Base> p_base = std::make_unique<Derived>(); // Polymorphic behavior with smart pointer

    // std::cout << "--- Exiting createAndDestroyObject() scope ---\n";
    // unique_ptr will call delete p_base; automatically here.
    // The problem persists because Base's destructor is NOT virtual, so Derived::~Derived() is not called.
}


int main() {
    const int num_iterations = 100; // Create 100 objects, leaking 100MB

    std::cout << "--- Starting memory leak demonstration (11MB leaked per object) ---\n";
    for (int i = 0; i < num_iterations; ++i) {
        createAndDestroyObject(i);
        if (i % 10 == 0) {
            std::cout << "Created " << i + 1 << " objects (leaked ~" << 11 * (i + 1) << "MB)...\n";
        }
    }

    std::cout << "\n--- Analysis ---\n";
    std::cout << "Observe that the 'Derived' destructor messages are NOT printed during the loop.\n";
    std::cout << "Even with std::unique_ptr, the memory allocated by std::vector<char> inside Derived is NOT freed\n";
    std::cout << "because Derived::~Derived() is never called due to the missing virtual destructor in Base.\n";
    std::cout << "This demonstrates a significant memory leak over time.\n";

    // Keep the program running briefly so you can observe memory usage
    // std::cout << "Press Enter to exit...";
    // std::cin.ignore();

    return 0;
}
