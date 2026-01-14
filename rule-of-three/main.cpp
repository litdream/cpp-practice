#include <iostream>
#include <cstring> // For std::strcpy

// A class that manages a raw C-style string resource.
// WARNING: This class is intentionally designed to be "bad" to demonstrate
// the problems that arise when the Rule of Three is violated.
class BadString {
public:
    // Constructor: allocates memory and copies the input string.
    BadString(const char* s = "") {
        std::cout << "Constructor called for '" << s << "'\n";
        size_ = std::strlen(s);
        data_ = new char[size_ + 1];
        std::strcpy(data_, s);
    }

    // Destructor: frees the allocated memory.
    ~BadString() {
        std::cout << "Destructor called for '" << (data_ ? data_ : "") << "'\n";
        delete[] data_;
        data_ = nullptr; // Good practice to null out dangling pointers.
    }

    // A simple method to print the string.
    void print() const {
        if (data_) {
            std::cout << data_ << std::endl;
        } else {
            std::cout << "[empty]" << std::endl;
        }
    }

private:
    char* data_;
    size_t size_;
    // The compiler implicitly generates:
    // 1. Copy Constructor: BadString(const BadString& other)
    // 2. Copy Assignment Operator: BadString& operator=(const BadString& other)
    // These default versions will only perform a *shallow copy* of data_,
    // which is the root of our problem.
};

void demonstrate_double_free() {
    std::cout << "\n--- Demonstrating Double Free ---\n";
    BadString original("original");

    // The compiler-generated copy constructor is called here.
    // It copies the pointer `data_`, not the actual data.
    // Now, `original` and `copy` both point to the SAME memory block.
    BadString copy = original;

    std::cout << "original.print(): ";
    original.print();
    std::cout << "copy.print(): ";
    copy.print();

    std::cout << "When this function ends, the destructors for both 'original' and 'copy' will be called.\n";
    std::cout << "Since they point to the same memory, the second destructor call will try to free memory that has already been freed, leading to a double-free bug (and likely a crash).\n";
}

int main() {
    demonstrate_double_free();

    std::cout << "\n--- End of main ---\n";
    // If the program hasn't crashed yet, you might see this message.
    // The actual crash might happen here or later, as double-free is undefined behavior.
    return 0;
}
