#include <iostream>
#include "ModbusClient.h"
#include <windows.h> // Required for Sleep() function

int main() {
    std::cout << "--- Modbus TCP Client Demo ---" << std::endl;

    ModbusClient modbus;
    
    // PLC Network Configuration
    std::string ipAddress = "192.168.1.100";
    int port = 502;
    uint8_t unitId = 1; // Default Slave ID for Modbus TCP

    std::cout << "Attempting to connect to PLC at " << ipAddress << ":" << port << "..." << std::endl;

    if (modbus.connect(ipAddress, port)) {
        std::cout << "Successfully connected!\n" << std::endl;

        // =========================================================
        // EXAMPLE 1: Write Single Coil (Turn ON/OFF a Relay/Buzzer)
        // =========================================================
        uint16_t coilAddress = 11; // Modbus memory address (e.g., MB0)

        std::cout << "[Test 1] Turning ON the coil at address " << coilAddress << "..." << std::endl;
        if (modbus.writeSingleCoil(unitId, coilAddress, true)) {
            std::cout << "-> Coil is now ON (Buzzer ringing!)." << std::endl;
        } else {
            std::cerr << "-> Failed to write to coil!" << std::endl;
        }

        // Wait for 2 seconds to observe the physical change
        Sleep(2000);

        std::cout << "Turning OFF the coil at address " << coilAddress << "..." << std::endl;
        modbus.writeSingleCoil(unitId, coilAddress, false);

        // =========================================================
        // EXAMPLE 2: Read Holding Registers
        // =========================================================
        uint16_t startAddress = 0;
        uint16_t quantity = 5; // We want to read 5 registers

        std::cout << "\n[Test 2] Reading " << quantity << " holding registers from address " << startAddress << "..." << std::endl;
        std::vector<uint16_t> registers = modbus.readHoldingRegisters(unitId, startAddress, quantity);

        if (!registers.empty()) {
            std::cout << "-> Read successful. Fetched Data:" << std::endl;
            for (size_t i = 0; i < registers.size(); ++i) {
                std::cout << "   Register[" << startAddress + i << "] : " << registers[i] << std::endl;
            }
        } else {
            std::cerr << "-> Failed to read registers!" << std::endl;
        }

        // Always disconnect when operations are complete
        modbus.disconnect();
        std::cout << "\nDisconnected from PLC. Demo finished." << std::endl;

    } else {
        std::cerr << "Connection failed! Please check the IP address, cable, and PLC status." << std::endl;
    }

    return 0;
}