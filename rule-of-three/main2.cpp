#include <iostream>
#include <cstring>

using std::cout;
using std::endl;

class BadString {
public:
    BadString(const char* s="") {
        cout << "Constructor for '" <<s <<'\n';
        size_ = std::strlen(s);
        data_ = new char[size_ + 1];
        std::strcpy(data_, s);
    }
    ~BadString() {
        cout << "Destructor for '" << (data_ ? data_ : "") << '\n';
        delete[] data;      // potentially double-free.
        data_ = nullptr;    // This is good practice.
    }
    // The compiler implicitly generates:
    // 1. Copy Constructor: BadString(const BadString& other)
    // 2. Copy Assignment Operator: BadString& operator=(const BadString& other)
    // These default versions will only perform a *shallow copy* of data_,
    // which is the root of our problem.
    
    void print() const {
        if (data_) 
            cout << data_ << endl;
        else
            cout << "[empty]" << endl;
    }
private:
    char* data_;
    size_t size_;
};

void demonstrate_double_free() {
    BadString original("original");
    BadString copy = original;

    original.print();
    copy.print();
}


int main() {
    demonstrate_double_free();

    cout << "\n -- END OF main() --\n";
    return 0;
}
