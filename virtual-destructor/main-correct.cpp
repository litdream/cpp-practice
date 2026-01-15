#include <iostream>
#include <memory> // For std::unique_ptr
#include <vector> // For std::vector

// --- Base Class ---
// This class NOW has a virtual destructor.
class Base {
public:
    Base() {
        // std::cout << "Base constructor called.\n";
    }
    // The 'virtual' keyword is the solution.
    virtual ~Base() {
        // std::cout << "Base destructor called.\n";
    }
};

// --- Derived Class ---
// Manages a resource (a dynamically allocated vector of chars).
class Derived : public Base {
public:
    static constexpr size_t RESOURCE_SIZE = 1024 * 1024; // 1MB per object

    Derived() {
        // std::cout << "Derived constructor called.\n";
        resource_ = std::vector<char>(RESOURCE_SIZE, 'B'); // Allocate a significant resource
    }
    // The 'override' keyword is good practice but not strictly required here.
    // It helps catch errors if the base class destructor wasn't actually virtual.
    ~Derived() override {
        // std::cout << "Derived destructor called. Resource (1MB) freed correctly.\n";
        // std::vector's destructor handles memory freeing, and it WILL be called polymorphically.
    }
private:
    std::vector<char> resource_;
};

// Function to demonstrate the correct behavior using unique_ptr within a scope
void createAndDestroyObjectCorrectly(int iteration) {
    // std::cout << "\n--- Entering createAndDestroyObjectCorrectly() scope (Iteration " << iteration << ") ---\n";
    // std::cout << "--- Creating Derived object via Base unique_ptr ---\n";
    std::unique_ptr<Base> p_base = std::make_unique<Derived>(); // Polymorphic behavior with smart pointer

    // std::cout << "--- Exiting createAndDestroyObjectCorrectly() scope ---\n";
    // unique_ptr will call delete p_base; automatically here.
    // Because Base's destructor IS virtual, the Derived destructor will be correctly called.
}


int main() {
    const int num_iterations = 100; // Create 100 objects, but memory should be freed each time

    std::cout << "--- Starting correct memory management demonstration (1MB allocated and freed per object) ---\n";
    for (int i = 0; i < num_iterations; ++i) {
        createAndDestroyObjectCorrectly(i);
        if (i % 10 == 0) {
            std::cout << "Created and destroyed " << i + 1 << " objects. Total memory should remain stable.\n";
        }
    }

    std::cout << "\n--- Analysis ---\n";
    std::cout << "Observe that memory usage should remain stable, unlike the 'main.cpp' example.\n";
    std::cout << "With std::unique_ptr and a virtual destructor in Base, the 'Derived' destructor is correctly called.\n";
    std::cout << "This ensures all resources are properly freed, demonstrating correct polymorphic destruction.\n";

    // std::cout << "Press Enter to exit...";
    // std::cin.ignore();

    return 0;
}
