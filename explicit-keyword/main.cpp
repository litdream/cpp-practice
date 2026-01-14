#include <iostream>
#include <string>
#include <vector>

// A simple class representing a buffer or a string-like object.
class Buffer {
public:
    // Non-explicit constructor: Allows implicit conversions from int.
    // This constructor is intended to create a buffer of a certain size.
    Buffer(int size) : size_(size) {
        data_.resize(size);
        std::cout << "  (Implicit) Buffer constructed with size: " << size_ << std::endl;
    }

private:
    int size_;
    std::vector<char> data_;
};

// A safer version of the Buffer class using the 'explicit' keyword.
class ExplicitBuffer {
public:
    // Explicit constructor: Prevents implicit conversions from int.
    explicit ExplicitBuffer(int size) : size_(size) {
        data_.resize(size);
        std::cout << "  (Explicit) ExplicitBuffer constructed with size: " << size_ << std::endl;
    }

private:
    int size_;
    std::vector<char> data_;
};

// A function that takes a Buffer object.
void processBuffer(const Buffer& buf) {
    std::cout << "Processing Buffer..." << std::endl;
    // In a real scenario, this function would do work with the buffer.
}

// A function that takes an ExplicitBuffer object.
void processExplicitBuffer(const ExplicitBuffer& buf) {
    std::cout << "Processing ExplicitBuffer..." << std::endl;
}


int main() {
    std::cout << "--- Demonstrating Non-Explicit Constructor (Bad Practice) ---" << std::endl;

    std::cout << "\n1. Direct initialization (this is always okay):" << std::endl;
    Buffer b1(10);

    std::cout << "\n2. Copy initialization (also okay):" << std::endl;
    Buffer b2 = 20; // An integer is implicitly converted to a Buffer!

    std::cout << "\n3. Function call with implicit conversion:" << std::endl;
    processBuffer(30); // An integer is implicitly converted to a temporary Buffer!

    std::cout << "\n4. Potentially buggy implicit conversion:" << std::endl;
    // 'a' is a char, which is promoted to an int (e.g., ASCII 97).
    // This is probably not what the programmer intended!
    processBuffer('a');


    std::cout << "\n\n--- Demonstrating Explicit Constructor (Good Practice) ---" << std::endl;

    std::cout << "\n1. Direct initialization (still okay):" << std::endl;
    ExplicitBuffer eb1(10);

    std::cout << "\n2. Copy initialization (now a compile error):" << std::endl;
    // ExplicitBuffer eb2 = 20; // COMPILE ERROR! No implicit conversion allowed.
    std::cout << "  Line `ExplicitBuffer eb2 = 20;` is commented out because it would fail to compile." << std::endl;


    std::cout << "\n3. Function call with implicit conversion (now a compile error):" << std::endl;
    // processExplicitBuffer(30); // COMPILE ERROR! No implicit conversion allowed.
     std::cout << "  Line `processExplicitBuffer(30);` is commented out because it would fail to compile." << std::endl;

    std::cout << "\n4. Correct, explicit way to call the function:" << std::endl;
    processExplicitBuffer(ExplicitBuffer(30)); // We must explicitly create an object.

    std::cout << "\n5. Preventing the buggy char-to-int conversion:" << std::endl;
    // processExplicitBuffer('a'); // COMPILE ERROR!
     std::cout << "  Line `processExplicitBuffer('a');` is commented out because it would fail to compile." << std::endl;


    std::cout << "\n\nConclusion:" << std::endl;
    std::cout << "The `explicit` keyword prevents the compiler from making potentially surprising\n" << 
              "and unintended type conversions, leading to safer and clearer code." << std::endl;

    return 0;
}
