#include <iostream>
#include <string>
#include <utility> // For std::move
#include <vector>

// A simple class that manages a resource (a piece of text).
class Message {
public:
    // Constructor
    Message(const std::string& text) : payload_(new std::string(text)) {
        std::cout << "Constructing Message: '" << *payload_ << "'\n";
    }

    // Destructor
    ~Message() {
        // The check for nullptr is crucial. Without it, deleting a moved-from
        // object would double-free memory.
        if (payload_) {
            std::cout << "Destructing Message: '" << *payload_ << "'\n";
        } else {
            std::cout << "Destructing a moved-from (empty) Message.\n";
        }
        delete payload_;
    }

    // Move Constructor
    // Note: It takes an R-value reference (Message&&)
    Message(Message&& other) noexcept : payload_(other.payload_) {
        std::cout << "Move Constructor called.\n";
        // After "stealing" the pointer, we set the source object's pointer
        // to nullptr. This is the "move". The source object is now in a
        // valid but empty state.
        other.payload_ = nullptr;
    }

    // For simplicity, we'll omit copy constructor, copy assignment,
    // and move assignment operators.

    void print() const {
        if (payload_) {
            std::cout << "Message payload: '" << *payload_ << "'\n";
        } else {
            std::cout << "Attempting to print an empty (moved-from) Message.\n";
        }
    }

    // A method that will cause a crash if called on a moved-from object.
    char getFirstChar() const {
        // *** DANGER ZONE ***
        // If payload_ is nullptr, this will dereference a null pointer.
        return (*payload_)[0];
    }

private:
    std::string* payload_;
};

int main() {
    Message original("Hello, World!");
    original.print();

    std::cout << "\n--- Moving the original message ---\n";

    // Here, we use std::move to cast `original` to an r-value reference,
    // which triggers the Move Constructor.
    Message destination = std::move(original);

    std::cout << "\n--- After the move ---\n";
    destination.print();
    original.print(); // This is safe, but shows the object is now empty.

    std::cout << "\n--- The Pitfall: Using the moved-from object ---\n";
    std::cout << "Attempting to access the first character of the original message...\n";

    // This is where the program will likely crash.
    // We are trying to use the `original` object after its payload_
    // has been moved to `destination`. `original.payload_` is now nullptr.
    try {
        char c = original.getFirstChar();
        std::cout << "First character of original: " << c << "\n";
    } catch (...) {
        // On some systems, this might be caught, but typically it's a segmentation fault.
        std::cout << "An exception was caught, but a crash is more likely.\n";
    }

    std::cout << "Program finished (or did it crash?).\n";

    return 0;
}
