#include <iostream>
#include <cstring> // For std::strlen, std::strcpy
#include <algorithm> // For std::swap

// A class that correctly manages a raw C-style string resource,
// adhering to the Rule of Three.
class MyString {
public:
    // Default Constructor
    MyString(const char* s = "") {
        std::cout << "Constructor called for '" << s << "'\n";
        size_ = std::strlen(s);
        data_ = new char[size_ + 1];
        std::strcpy(data_, s);
    }

    // Destructor
    ~MyString() {
        std::cout << "Destructor called for '" << (data_ ? data_ : "") << "'\n";
        delete[] data_;
        data_ = nullptr;
    }

    // Copy Constructor (Rule of Three)
    // Performs a deep copy of the string data.
    MyString(const MyString& other) {
        std::cout << "Copy Constructor called for '" << (other.data_ ? other.data_ : "") << "'\n";
        size_ = other.size_;
        data_ = new char[size_ + 1];
        std::strcpy(data_, other.data_);
    }

    // Copy Assignment Operator (Rule of Three)
    // Implemented using the copy-and-swap idiom for strong exception safety.
    MyString& operator=(const MyString& other) {
        std::cout << "Copy Assignment called for '" << (data_ ? data_ : "") << "' from '" << (other.data_ ? other.data_ : "") << "'\n";
        if (this != &other) { // Protect against self-assignment
            // Create a temporary copy of 'other'
            MyString temp(other); 
            // Swap the contents of 'this' with the temporary
            std::swap(data_, temp.data_);
            std::swap(size_, temp.size_);
            // When 'temp' goes out of scope, its destructor frees the old memory of 'this'
        }
        return *this;
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
};

void demonstrate_rule_of_three() {
    std::cout << "\n--- Demonstrating Rule of Three (Correct Behavior) ---\n";
    MyString s1("Hello");
    std::cout << "s1: ";
    s1.print();

    MyString s2 = s1; // Calls copy constructor
    std::cout << "s2 (copy of s1): ";
    s2.print();

    MyString s3("World");
    std::cout << "s3: ";
    s3.print();

    s3 = s1; // Calls copy assignment operator
    std::cout << "s3 (assigned from s1): ";
    s3.print();

    std::cout << "\n--- Modifying s1 and observing s2 and s3 (deep copies) ---\n";
    // To modify s1, we need to add a setter or reassign. For simplicity,
    // let's re-create s1 to show independence.
    s1 = MyString("Changed"); 
    std::cout << "s1 (after change): ";
    s1.print();
    std::cout << "s2 (should be original 'Hello'): ";
    s2.print();
    std::cout << "s3 (should be original 'Hello'): ";
    s3.print();

    std::cout << "When this function ends, destructors for s1, s2, and s3 will be called.\n";
    std::cout << "Each object manages its own distinct memory, so there will be no double-free.\n";
}

int main() {
    demonstrate_rule_of_three();

    std::cout << "\n--- End of main-correct ---\n";
    return 0;
}
