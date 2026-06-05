# GEMINI.md

An Architectural Blueprint for an Apple II Computer Emulator in C++: Deconstructing a Working Implementation into Educational Modules.

---

## 1. Executive Summary & Methodology

This project details the architectural blueprint for a book designed to teach systems programming, computer architecture, and emulation mechanics through a unique **"reverse-engineering for pedagogy"** approach. 

Rather than building an emulator from scratch through a blind process of trial-and-error, this book begins with a verified, fully operational production asset: a 6502-based Apple II emulator written in native C++ (located within the `./apple2-emulator-cpp` subdirectory). This emulator was successfully synthesized with the assistance of Gemini, navigating the typical architectural pitfalls, timing mismatches, and hardware edge cases common to systems emulation.

The core educational philosophy of this curriculum is **Deconstruction**. We systematically disassemble a monolithic, fully working artifact into 10 structured, bite-sized, and highly didactic chapters. By starting with a known-good baseline, students avoid the existential frustration of debugging compound bugs across multiple unverified hardware subsystems (e.g., debugging an invalid CPU instruction handler while simultaneously diagnosing an unmapped memory bus error).

Provide Code snapshot for each chapter.  So that students can feel like joy of building software.

---

## 2. Book Syllabus & Structural Outline

### Chapter 1: Introduction, Architecture, and Foundations
* **Core Objective:** Establish the historical framework of the Apple II computer, delineate the target hardware specification, and initialize the development environment.
* **The Paradigm Shift:** Traditional systems programming courses rely heavily on writing abstract software layers. This chapter contextualizes the physical reality of 1977 computing hardware.
* **Historical & Reference Material:** Guidelines on navigating original MOS 6502 data sheets, the Apple II Reference Manual ("The White Book"), and Wozniak’s original monitors.
* **Toolchain Selection:** Native C++ (C++17/20 standard) paired with **SDL2 (Simple DirectMedia Layer)** for high-performance pixel-buffer rendering and hardware input handling.
* **The "Bootstrap" Strategy:** To accelerate learning and bypass the grueling process of manual opcode matrix verification, we purposefully ingest three highly stable, production-tested headers extracted directly from the open-source **AppleWin** project:
    * `apple2plus_rom.h`: The standard 12KB system ROM containing the Applesoft BASIC interpreter and System Monitor.
    * `CPU6502_opcodes.h`: A flawless, pre-audited matrix mapping 6502 opcodes to execution functions.
    * `CPU6502_switch.h`: The execution dispatch mechanism handling standard, undocumented, and cycle-exact timing branching.
* **Pedagogical Safeguard:** By inheriting these verified components, students eliminate months of tedious debugging and focus exclusively on the core systems architecture: memory layout, bus routing, and hardware synchronization.

### Chapter 2: The Skeleton Frame – Initializing SDL2
* **Core Objective:** Construct an isolated, minimalist multimedia wrapper capable of rendering an empty window, capturing system ticks, and establishing a baseline execution loop.
* **Technical Focus:** Configuring SDL2 subsystems (`SDL_INIT_VIDEO`, `SDL_INIT_EVENTS`), spawning an `SDL_Window`, and attaching an `SDL_Renderer`.
* **The Blueprint:** Students build a foundational "heartbeat" mechanism. This structure introduces the concept of the emulation master loop before it is complicated by CPU states or memory cycles.

### Chapter 3: Memory Subsystem Architecture
* **Core Objective:** Modeling physical RAM, ROM, and Memory-Mapped I/O (MMIO) spaces inside a modern, contiguous virtual environment.
* **Data Structures:** Moving away from dangerous raw pointers to safe, bound-checked `std::vector<uint8_t>` or `std::array<uint8_t, 65536>` representations of the standard 64KB addressing space.
* **Hardware Concepts Detailed:**
    * **Paging:** Breaking the 16-bit address space ($0000 to $FFFF) into 256-byte structural chunks.
    * **Zero-Page ($0000–$00FF):** Explaining why the MOS 6502 treats the first 256 bytes of memory as an ultra-fast internal register array, and how to optimize host-side lookups for zero-page addressing modes.
    * **Stack Page ($0100–$01FF):** Hardcoded physical boundaries of the 6502 stack.
    * **Display Pages:** Sectioning RAM arrays reserved for Text/Low-Res Graphics (Page 1: $0400–$07FF) and High-Res Graphics.

### Chapter 4: The Bus – The System Interconnect
* **Core Objective:** Implement the communication backplane that binds the CPU, Memory, and I/O devices together.
* **The Theoretical Shift:** In physical hardware, the system bus is an array of copper traces carrying voltage highs and lows. In C++, we must translate this parallel physical infrastructure into a clean object-oriented abstraction or functional routing matrix.
* **Implementation Mechanics:** Designing standard `Bus::read(uint16_t address)` and `Bus::write(uint16_t address, uint8_t data)` entry points. This section demonstrates how software decodes addresses to determine whether a read/write operation is routed to systemic RAM, configuration ROM, or memory-mapped I/O switches.

### Chapter 5: The Central Processing Unit (MOS 6502 Engine)
* **Core Objective:** Encapsulate the execution state, internal registers, flags, and Arithmetic Logic Unit (ALU) execution patterns of the 6502 processor.
* **State Matrix:** Implementing internal registers: Program Counter (`PC`), Stack Pointer (`S`), Accumulator (`A`), X Register (`X`), Y Register (`Y`), and the Status Register (`P`).
* **Execution Cycle Topology:** Mapping out the strict structural dance of hardware execution:
    1. **Fetch:** Retrieving the next opcode from the Bus using the `PC`.
    2. **Decode:** Passing the opcode into our imported `CPU6502_switch.h` structure to identify instructions and address resolution pathways.
    3. **Execute:** Mutating state registers, processing ALU calculations, and advancing the execution clock cycle counters.

### Chapter 6: Video Architecture & Font Rasterization
* **Core Objective:** Emulate the discrete logic video generation circuit of the Apple II, translating raw memory ranges into visible pixels.
* **Technical Specification:** Deconstructing the Apple II primary text map ($0400–$07FF). 
* **Font Engine Construction:** Creating a localized software font renderer. We explore how character matrices are mapped using binary font tables, how individual bytes are translated into a 7x8 bit array, and how to map these arrays directly into an SDL text generation engine matching original NTSC styling.

### Chapter 7: ROM Loading & Environment Bootstrap
* **Core Objective:** Inject the core system operating parameters into the emulator's upper memory spaces.
* **Mechanics:** Utilizing the pre-compiled `apple2plus_rom.h` byte arrays. Students learn to map these binary images directly into the virtual memory bus space starting at the historical upper boundary ($D000 to $FFFF). This simulates flashing physical EEPROMs on a motherboard.

### Chapter 8: Power Up and the System Reset Vector
* **Core Objective:** Simulate the electrical initialization sequence of the machine.
* **Technical Deep-Dive:** When a physical Apple II powers up or its reset key is pressed, the CPU executes a hardcoded microcode routine. It queries the memory addresses `$FFFC` and `$FFFD` to read a 16-bit destination address known as the **Reset Vector**.
* **The Milestone:** Students call `cpu.reset()`, which forces the CPU to jump to the address dictated by the newly loaded ROM. This initializes the system monitor, clears memory, clears the screen, prints the iconic `Apple ][` banner, and displays the Applesoft BASIC `]` command prompt awaiting interactive input.

### Chapter 9: The Interactive Loop & Event Integration
* **Core Objective:** Wire host-system human interface inputs (keyboard, control vectors) into the emulated machine state and continuously refresh the physical screen display.
* **The Data Loop:**
    * Intercepting host OS hardware keyboard events via `SDL_PollEvent`.
    * Translating modern ASCII keys to Apple II hardware strobes, mutating memory-mapped keyboard registers (`$C000`), and clearing the strobe state via read operations at `$C010`.
    * Synchronizing the execution timing loop to ensure the emulated 6502 runs at its historical speed (~1.023 MHz) rather than consuming 100% of modern multi-GHz host CPU cycles.

### Chapter 10: Advanced Debugging and System Inspection Mechanics
* **Core Objective:** Build developer-centric introspection instrumentation directly inside the emulation core.
* **Features:**
    * **Instruction Disassembler:** Converting execution streams back into readable assembly text blocks (e.g., `A9 01 -> LDA #$01`).
    * **State Inspection Panel:** Real-time printing of registry values (`A`, `X`, `Y`, `PC`, `S`) and condition flags to stdout or an auxiliary UI panel.
    * **Memory Hex-Dumper:** Creating hooks to read full pages of virtual system RAM during runtime execution to track real-time changes inside the Zero-Page.

---

## 3. Future Enhancements & Subsystem Expansion
Once the baseline 10-chapter architecture is completely operational, the book introduces supplemental modules to expand the fidelity of the emulator:

1.  **Audio Subsystem (The Apple II Speaker):** Capturing explicit toggle writes to memory address `$C030`. Converting these erratic software state toggles into square-wave audio frequencies using the `SDL_QueueAudio` API to reproduce classic retro game clicks, ticks, and multi-tone signals.
2.  **Storage Subsystem (Disk II Controller):** Emulating the structural wonders of Steve Wozniak's Disk II controller card. Implementing the state machine for the Group Code Recording (GCR) format, parsing `.do` or `.dsk` virtual floppy images, tracking virtual step motors, and streaming sector data directly into RAM via slots.
3.  **Expansion Slots & Peripheral Emulation:** Modeling the classic 8-slot physical backplane configuration ($C100–$C7FF) to allow modular additions like 80-column text cards or mockingsound engines.

---

## 4. Verification Checkpoint
The structural integrity of this syllabus is guaranteed by the physical reference implementation stored within:
`./apple2-emulator-cpp

This subdirectory acts as the definitive source of truth for all code blocks, logic flows, and hardware integration maps outlined throughout the chapters of this book.




===
# Version 1 was made.  (./v1)

Version 1 is good.
But, this version doesn't show good SDL2 output.
The whole purpose of the book is, to build SDL2 application (doing apple 2 emulator).   We did emulator part. 

So, let's remake Version 2 (./v2), based on Version 1.  But, this time, presentaion layer is not text (stdout), but SDL2, so that user feel like, they are building actual computer.

===
# Version 2 was made.  (./v2)

Version 2 is complete! We have migrated the presentation layer in chapters 6, 8, 9, and 10 from terminal headless operations to true hardware-accelerated SDL2 window outputs with high-fidelity video rasterization, fulfilling the original pedagogy of building an actual emulated computing visual interface.

