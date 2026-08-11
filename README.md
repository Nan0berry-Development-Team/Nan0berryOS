# Nan0berryOS

**Nan0berryOS** is a high-performance, minimalist Real-Time Operating System (RTOS) kernel built entirely from scratch specifically for the **Raspberry Pi Pico (RP2040)** and ARM Cortex-M0+ architecture. It delivers a lightweight preemptive runtime, a persistent Flash file system, and an interactive serial shell designed for low-overhead embedded applications.

---

## 🌟 Key Features

- **Low-Level Preemptive Kernel:** Custom context-switching implementation via ARM Cortex-M0+ `PendSV_Handler` in Assembly, ensuring safe register preservation (`R4-R7`) and proper 8-byte stack alignment.
- **Persistent Flash File System (`nano_fs`):** Built-in non-volatile storage mapping directly to SPI Flash, allowing files to persist across reboots with integrated sector erasure and interrupt masking (`save_and_disable_interrupts`).
- **Interactive CLI Shell:** Real-time command-line interface (`nano_cli_process`) over USB CDC supporting direct file management utilities (`ls`, `cat`, `write`).
- **Synchronization & Concurrency:** Priority-based task scheduling combined with atomic mutex primitives for safe resource sharing across threads.
- **RP2040 Optimization:** Fully integrated with the official Raspberry Pi Pico SDK, utilizing modern CMake and Ninja build pipelines.

---

## 📂 Project Structure

- `main.c` - Application entry point, task initialization, and default shell/blink routines.
- `nanoberry.c` - Core kernel logic, file system manager, and CLI command processor.
- `nanoberry.h` - Kernel definitions, system limits, and public API headers.
- `port_asm.S` - Low-level assembly routines for ARM Cortex-M0+ context switching (`PendSV_Handler`).
- `CMakeLists.txt` - Project build configurations and SDK bindings.
- `releases/` - Packaged release binaries ready for direct deployment (`.uf2`).

---

## ⚙️ Getting Started

### Prerequisites

- [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk) installed and configured (`PICO_SDK_PATH`)
- ARM GNU Toolchain (`arm-none-eabi-gcc`)
- CMake (version 3.13 or newer)
- Ninja build system

### Setup

1. Clone the repository into your workspace:
   ```bash
   git clone [https://github.com/Nan0berry-Development-Team/Nan0berryOS.git](https://github.com/Nan0berry-Development-Team/Nan0berryOS.git)
   cd Nan0berryOS
Set up your local Pico SDK environment variables as required by your toolchain.

🛠️ Building from Source
Configure the build directory using Ninja:

Bash
cmake -G Ninja -S . -B build
Compile the project:

Bash
cmake --build build
⚡ Flashing
After building, you can flash the compiled binary directly to your Raspberry Pi Pico:

Press and hold the BOOTSEL button on your Pico.

Connect the Pico to your PC via USB.

Drag and drop the generated binary (releases/Nan0berryOS.uf2 or from your build/ folder) directly onto the Pico mass storage drive.

🗺️ Roadmap & Future Updates
[ ] Expand task sleep and advanced blocking primitives (nano_delay_ms enhancements)

[ ] Implement inter-task messaging and queues

[ ] Add dynamic priority boosting mechanisms

[ ] Support extended hardware peripherals and additional board variants

📜 License
This project is released under the MIT License. See the LICENSE file for details.
