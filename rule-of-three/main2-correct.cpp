#include <iostream>
#include <cstring>     // strlen, strcpy
#include <algorithm>   // std::swap

using std::cout;
using std::endl;

class MyString {
public:
    MyString(const char* s="") {
        cout << "Constructor for '" << s << '\n';
        size_ = std::strlen(s);
        data_ = new char[size_ + 1];
        std::strcpy(data_, s);
    }
    ~MyString() {
        cout << "Destructor for '" << (data_? data_ : "") << '\n';
        delete[] data_;
        data_ = nullptr;   // best practice;
    }

    // Copy Const
    MyString(const MyString& other) {
        cout << "Copy Ctor for '" << (other.data_ ? other.data_ : "") << endl;
        size_ = other.size_;
        data_ = new char[size_ + 1];
        std::strcpy(data_, other.data_);
    }

    // Copy Assignment Operator
    MyString& operator=(const MyString& other) {
        cout << "Copy Assignment for '" << (data_ ? data_ : "") << "' from '" << (other.data_? other.data_ : "") << '\n';
        if (this != &other)  {    // protect against self-assignment  (this can happen!)
            MyString temp(other);
            std::swap(data_, temp.data_);
            std::swap(size_, temp.size_);
        } // temp will call destructors,  WHICH Contains this's old pointers.
        return *this;
    }

    void print() const {
        if (data_)
            cout << data_ << endl;
        else
            cout << "[empty]" <<endl;
    }
private:
    char* data_;
    size_t size_;
};

void demonstrate_rule_of_three() {
    MyString s1("Hello");
    MyString s2 = s1;
    MyString s3("World");

    s3 = s1;     // assignment test.
    s3.print();

    s1 = MyString("Changed");
    s1.print();
    s2.print();   // should be orig
    s3.print();  

    // At this point, destructor for s1,s2,s3 will be called.
    // no-double-delete should happen
}

int main() {
    demonstrate_rule_of_three();
    return 0;
}
