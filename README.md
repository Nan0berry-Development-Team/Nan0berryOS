# Nan0berryOS

Nan0berryOS is a lightweight, custom real-time operating system kernel designed for the Raspberry Pi Pico (RP2040) and ARM Cortex-M0+ embedded platform. It provides a minimal preemptive multitasking runtime, a custom scheduler, and low-overhead context switching for bare-metal Pico applications.

## Key Features

- Preemptive multitasking with task switching via PendSV
- Lightweight context save/restore for ARM Cortex-M0+
- RP2040-specific build support using the official Raspberry Pi Pico SDK
- Custom scheduler architecture optimized for small embedded systems
- USB stdio support for debug output on the Pico

## Project Structure

- `src/` - Core application and kernel source code
- `src/kernel/` - Nan0berry kernel implementation and Cortex-M0+ context switch assembly
- `build/` - CMake/Ninja build output and generated artifacts (ignored by git)
- `releases/` - Packaged release binaries meant for distribution
- `CMakeLists.txt` - Project build configuration for Raspberry Pi Pico SDK
- `.gitignore` - Files and directories excluded from version control

## Getting Started

### Prerequisites

- Raspberry Pi Pico SDK installed and configured
- ARM GNU Toolchain for `arm-none-eabi`
- CMake 3.13 or newer
- Ninja build system

### Setup

1. Clone the repository:
   ```bash
   git clone https://github.com/<your-username>/Nan0berryOS.git
   cd Nan0berryOS
   ```
2. Configure the Pico SDK environment as required by your machine.

## Building from Source

Configure the build directory:
```bash
cmake -G Ninja -S . -B build
```

Build the project:
```bash
cmake --build build
```

## Flashing

After building, copy the generated `.uf2` file to your Pico while holding the BOOTSEL button. The generated release binary is also available in `releases/`.

1. Press and hold the BOOTSEL button on the Pico.
2. Connect the Pico to your PC via USB.
3. Copy `releases/Nan0berryOS_v0.1.uf2` to the Pico mass storage device.

## License

This project is released under the MIT License. See `LICENSE` for details.

## Roadmap

- Add task sleep and blocking primitives
- Implement inter-task messaging and synchronization objects
- Add a configurable priority scheduler
- Support additional RP2040 board variants and hardware peripherals
