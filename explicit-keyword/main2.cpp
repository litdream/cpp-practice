#include <iostream>
#include <string>
#include <vector>

using std::cin;
using std::cout;
using std::endl;

// So, Buffer and ExplicitBuffer are same,
//   - except: how to call constructor.
//
class Buffer;
class ExplicitBuffer;

class Buffer {
public:
    Buffer(int size) : size_(size) {
        data_.resize(size);
        cout << "  (Implicit) Buffer constructed with size: " << size_ << endl;
    }
private:
    int size_;
    std::vector<char> data_;
};

class ExplicitBuffer {
public:
    explicit ExplicitBuffer(int size) : size_(size) {
        data_.resize(size);
        cout <<"  (Explicit) ExplicitBuf constructed size: " << size_ <<endl;
    }
private:
    int size_;
    std::vector<char> data_;
};


void processBuffer(const Buffer& buf) {
    cout << "Proc Buff:  " << endl;
}

void processExplicitBuffer(const ExplicitBuffer& buf) {
    cout << "Proc ExplicitBuf:  " << endl;
}

int main() {
    Buffer b1(10);
    Buffer b2 = 20;
    processBuffer(30);     // int  is passed!  This makes temporary object.
    processBuffer('a');    // char is passed!  which promotes to int.
        // This will create a temporary Buffer, size of 97('a'), not "Process char 'a' in buffer."
        // Implicitly converted 'a' -> Buffer(int) object.

    ExplicitBuffer eb1(10);
    // ExplicitBuffer eb2 = 20;    // Will FAIL compile.

    // processExplicitBuffer(30);   // COMPILE ERR.   No implicit conversion allowed.
    processExplicitBuffer(ExplicitBuffer(30));  // But we can make a temporary object, and it will work.
    
}
