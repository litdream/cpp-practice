#include <iostream>
#include <string>
#include <utility>  // for std::move
#include <vector>

using std::cout;
using std::endl;

class Message {
public:
    Message(const std::string& text) : payload_(new std::string(text)) {
    }
    ~Message() {
        if (payload_)
            cout << "Destructing Message: " << *payload_ << endl;
        else
            cout << "Destructing Empty 'payload_' message.\n";
        delete payload_;
    }

    // Move Constructor
    //   - using R-value reference (Message&&)
    Message(Message&& other) noexcept : payload_(other.payload_) {
        cout << "Move Constructor called:  (stealing pointer).\n";
        other.payload_ = nullptr;
    }

    void print() const {
        if (payload_) 
            cout << "Message: " << *payload_ << endl;
        else
            cout << "Attempting to print an empty (moved-from) message\n";
    }

    // Will Crash the program, intentionally.
    char getFirstChar() const {
        return (*payload_)[0];
    }

private:
    std::string* payload_;
};

int main() {
    Message original("Hello, World!");
    original.print();

    Message destination = std::move(original);
    //  - Now, message has moved.

    destination.print();
    original.print();

    cout << "now, move finished, and accessing 'moved-from' object." <<endl;

    try {
        char c = original.getFirstChar();
        cout << "First char: " << c << '\n';
    } catch (...) {
        cout << "EXCEPTION occurred.\n";
    }

    cout << "Program finished.\n";
    return 0;
}
