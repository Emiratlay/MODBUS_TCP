# Simple Modbus TCP Client in C++

A lightweight, Object-Oriented Modbus TCP client library written in modern C++ for Windows environments. It uses the native `Winsock2` API, meaning it requires **zero external dependencies** (no Boost, no Qt, etc.).

## Features

- **Pure C++11:** Clean, readable, and well-structured OOP architecture.
- **No External Libraries:** Uses native Windows Sockets (`Ws2_32.lib`).
- **Standard Modbus TCP:** Fully implements the MBAP header and big-endian conversions automatically.

### Supported Function Codes

| Code | Function Name | Description |
| :--- | :--- | :--- |
| `0x01` | **Read Coils** | Read digital outputs (bits) |
| `0x02` | **Read Discrete Inputs** | Read digital inputs (bits) |
| `0x03` | **Read Holding Registers** | Read read/write analog 16-bit registers |
| `0x04` | **Read Input Registers** | Read read-only analog 16-bit registers |
| `0x05` | **Write Single Coil** | Write to a single digital output |
| `0x06` | **Write Single Register** | Write to a single 16-bit register |
| `0x10` | **Write Multiple Registers** | Write to a block of 16-bit registers |

## Getting Started

### Prerequisites
- Windows OS
- A C++ Compiler (GCC/MinGW or MSVC)

### Compilation
If you are using **GCC (MinGW)** via command line or VSCode terminal, you must link the Winsock library using the `-lws2_32` flag:

```bash
g++ *.cpp -o modbus_app.exe -lws2_32

## 🛠️ How the Functions Work (API Reference)

In Modbus, data is divided into four main tables. Our library handles all the bit-shifting and byte conversions for you. Here is how the specific functions interact with these tables:

### 1. Bit (Boolean) Operations
* **Coils (Read/Write):** Used to control digital outputs like relays, valves, or alarms.
* **Discrete Inputs (Read Only):** Used to read digital inputs like buttons, switches, or sensors.

```cpp
// 0x01 - Read Coils
std::vector<bool> readCoils(uint8_t unitId, uint16_t startAddress, uint16_t quantity);

// 0x02 - Read Discrete Inputs
std::vector<bool> readDiscreteInputs(uint8_t unitId, uint16_t startAddress, uint16_t quantity);
