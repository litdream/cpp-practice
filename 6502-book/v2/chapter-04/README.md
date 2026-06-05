# Chapter 4: The Bus – The System Interconnect

## 📖 Overview & Educational Philosophy
In Chapter 4, we establish the backplane architecture that binds our CPU, Memory subsystems, and hardware peripherals together. 

In physical computers, the system bus is an array of parallel copper lines carrying binary voltages. In C++, we translate this architectural layout into an Object-Oriented routing matrix using an abstract base class (`Bus.h`) and our concrete `SystemBus.cpp` interconnect.

## 🛠️ Memory-Mapped I/O (MMIO) Interception
The real magic of the `SystemBus` is its ability to decode addresses and determine which physical device should handle a given read or write request. 

While the majority of our 64KB addressing space routes straight to the `Memory` subsystem implemented in Chapter 3, addressing ranges mapped within `$C000` to `$CFFF` are intercepted directly by our `SystemBus` to poll:
* **`$C000`**: Apple II Keyboard Keystrobe Latch.
* **`$C010`**: Keyboard Strobe Clear (Acknowledge).
* **`$C030`**: Hardware Audio Speaker Tick (`[BEEP]`).

## ⚙️ Building and Running Chapter 4
Our `main.cpp` executes an automated test suite verifying that routing through the `SystemBus` accurately delegates to system RAM while properly intercepting, latching, and clearing I/O keystrobes.

### Build Steps:
```bash
mkdir build
cd build
cmake ..
make
./chapter-04
```
