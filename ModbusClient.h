#ifndef MODBUS_CLIENT_H
#define MODBUS_CLIENT_H

#include "TcpSocket.h"
#include "ModbusDefinitions.h"
#include <vector>
#include <cstdint>
#include <string>

class ModbusClient {
public:
    ModbusClient();
    ~ModbusClient();

    // 1. Köprü Fonksiyonlar (Sadece içerdeki soketi çağırırlar)
    bool connect(const std::string& ipAddress, int port = Modbus::DEFAULT_PORT);
    void disconnect();
    bool isConnected() const;

    // 2. Modbus Kullanıcı Fonksiyonları (Function Codes)
    // Örnek: 0x03 - Read Holding Registers
    std::vector<uint16_t> readHoldingRegisters(uint8_t unitId, uint16_t startAddress, uint16_t quantity);
    
    // İleride buraya writeSingleRegister vb. eklenecek...
    // 0x05 - Write Single Coil (Bite 1 veya 0 yazma)
    bool writeSingleCoil(uint8_t unitId, uint16_t address, bool value);
    // 0x02 - Read Discrete Inputs (Sadece okunabilir bitler)
    std::vector<bool> readDiscreteInputs(uint8_t unitId, uint16_t startAddress, uint16_t quantity);

    // 0x04 - Read Input Registers (Sadece okunabilir 16-bit analog veriler)
    std::vector<uint16_t> readInputRegisters(uint8_t unitId, uint16_t startAddress, uint16_t quantity);

    // 0x06 - Write Single Register (Tek bir 16-bit veriye yazma)
    bool writeSingleRegister(uint8_t unitId, uint16_t address, uint16_t value);

    // 0x10 - Write Multiple Registers (Birden fazla 16-bit veriye blok halinde yazma)
    bool writeMultipleRegisters(uint8_t unitId, uint16_t startAddress, const std::vector<uint16_t>& values);
private:
    TcpSocket socket;                // Ağ ile haberleşecek işçimiz
    uint16_t transactionIdCounter;   // Her istekte artacak olan sayaç

    // 3. Gizli Silahımız: Paket Birleştirici
    std::vector<uint8_t> buildRequestPacket(uint8_t unitId, Modbus::FunctionCode functionCode, const std::vector<uint8_t>& payload);
};

#endif // MODBUS_CLIENT_H