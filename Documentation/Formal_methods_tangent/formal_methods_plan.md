# HAMPOD Formal Methods Idea Draft
> **NOTE:** This file represents a collection of ideas for a potential future implementation of formal methods in the HAMPOD project. The plans herein are speculative and not currently scheduled for implementation. They serve as a "north star" for ensuring high reliability in the future.

## 1. Where to Start?

Applying formal methods to an existing embedded system like HAMPOD is best done incrementally. You should not try to "prove the whole system" at once.

### Recommended Starting Point: The Communication Protocol
The most critical and error-prone part of the system is the **Firmware <-> Software Pipe Communication Protocol**.
*   **Why:** Concurrency, race conditions, and deadlocks are notoriously hard to test via standard methods (as seen with the recent "zombie process" and "connection bugs").
*   **Goal:** Formally specify the packet exchange to prove that no sequence of events can cause the system to hang or desync.

### Secondary Starting Point: The User Interface State Machine
The "Hampod" user experience is effectively a large Hierarchical State Machine (Modes, Menus, Input States).
*   **Why:** Ensuring the user never gets "stuck" in a mode and that every input has a deterministic result.
*   **Goal:** Model the transition logic (e.g., "Frequency Entry Mode") to verify that all inputs (valid numeric, invalid, backspace, mode switching) are handled correctly.

---

## 2. Properties and Behaviors to Prove

### Safety Properties (Bad things never happen)
1.  **Deadlock Freedom**: The Firmware and Software never wait on each other indefinitely.
2.  **Buffer Safety**: The C-based Firmware never reads/writes partially formed packets (buffer overflows/underflows).
3.  **State Validity**: The system never enters an undefined Mode or State.

### Liveness Properties (Good things eventually happen)
1.  **Responsiveness**: If a key is pressed (and hardware works), an Audio response is *always* generated eventually.
2.  **Progress**: A firmware update or mode change command always completes or times out safely (never hangs).

### Functional Correctness ( The system does what it should)
1.  **Mode Logic**: "If in Frequency Mode, pressing '1' then 'Enter' results in a Hamlib command to set frequency to '1' (plus logic for Hz/kHz)."
2.  **Hardware Abstraction**: "The HAL `read()` function always returns a valid event or a 'no-event' code, never garbage."

---

## 3. Appropriate Tools for this Environment

Given the stack (C code on Raspberry Pi, Inter-Process Communication via Pipes, potential future rewrites):

### A. High-Level Design & Concurrency: **TLA+**
*   **Use Case:** Modeling the Pipe Protocol and the "Fresh Start" architecture before writing code.
*   **Benefit:** Excellent for finding race conditions in the communication layer.
*   **Effort:** Moderate. You write a "spec" (not code) that models the logic.

### B. Protocol Verification: **Spin (Promela)**
*   **Use Case:** Specifically verifying the message passing protocol between Firmware and Software.
*   **Benefit:** Simpler to write for C-programmers than TLA+. perfect for "If I send A, then B must happen."

### C. Code-Level Verification: **Frama-C**
*   **Use Case:** verifying the C firmware code (`hal_keypad.c`, `firmware.c`).
*   **Benefit:** Can prove absence of buffer overflows and runtime errors.
*   **Integration:** You add special comments (ACSL) to your C files. `/*@ requires valid_buffer(b); */`.
*   **Relevance:** Highly relevant for the `Firmware/` directory to ensure rock-solid stability.

### D. State Machine Modeling: **Statecharts / XState** (Tangential)
*   **Use Case:** Visualizing and verifying the logic of the Input Modes.
*   **Benefit:** Helps ensure the UI logic covers all edge cases (e.g., "What if I press Backspace when the buffer is empty?").

---

## 4. Practical Roadmap (Speculation)

If we were to apply this:
1.  **Phase 0**: Write a TLA+ spec for the Pipe Protocol (Firmware <-> Software handshakes).
2.  **Phase 1**: Annotate critical `hal_*.h` interfaces with Frama-C contracts to define exactly what the hardware layer promises.
3.  **Phase 2**: Model the complex "Frequency Entry Mode" logic to ensure all edge cases are handled before coding the new Software layer.
