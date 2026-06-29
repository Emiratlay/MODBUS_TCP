#include "ModbusClient.h"
#include <iostream>

ModbusClient::ModbusClient() : transactionIdCounter(0) {
    // Sınıf oluşturulduğunda sayaç sıfırdan başlar
}

ModbusClient::~ModbusClient() {
    disconnect();
}

bool ModbusClient::connect(const std::string& ipAddress, int port) {
    return socket.connectTo(ipAddress, port);
}

void ModbusClient::disconnect() {
    socket.disconnect();
}

bool ModbusClient::isConnected() const {
    return socket.getIsConnected();
}

// GİZLİ SİLAH: MBAP Başlığını oluşturan ve paketi birleştiren yardımcı fonksiyon
std::vector<uint8_t> ModbusClient::buildRequestPacket(uint8_t unitId, Modbus::FunctionCode functionCode, const std::vector<uint8_t>& payload) {
    std::vector<uint8_t> packet;

    // 1. Transaction ID (2 Byte - Big Endian)
    packet.push_back((transactionIdCounter >> 8) & 0xFF); // High Byte
    packet.push_back(transactionIdCounter & 0xFF);        // Low Byte
    transactionIdCounter++; // Bir sonraki istek için sayacı artır

    // 2. Protocol ID (2 Byte - Modbus TCP için her zaman 0x0000)
    packet.push_back(0x00);
    packet.push_back(0x00);

    // 3. Length (2 Byte - Bundan sonraki byte'ların toplam sayısı)
    // Uzunluk = 1(UnitID) + 1(FuncCode) + Payload Uzunluğu
    uint16_t length = 1 + 1 + payload.size();
    packet.push_back((length >> 8) & 0xFF);
    packet.push_back(length & 0xFF);

    // 4. Unit ID (1 Byte)
    packet.push_back(unitId);

    // 5. Function Code (1 Byte)
    packet.push_back(static_cast<uint8_t>(functionCode));

    // 6. Payload (Veri Yükü) - Vektörün sonuna ekle
    packet.insert(packet.end(), payload.begin(), payload.end());

    return packet;
}

// KULLANICI FONKSİYONU: 0x03 Read Holding Registers
std::vector<uint16_t> ModbusClient::readHoldingRegisters(uint8_t unitId, uint16_t startAddress, uint16_t quantity) {
    if (!isConnected()) {
        std::cerr << "Hata: Baglanti yok!" << std::endl;
        return {};
    }

    // İstek (Request) Payload'ını oluştur (Adres ve Miktar)
    std::vector<uint8_t> payload;
    payload.push_back((startAddress >> 8) & 0xFF); // Adres High
    payload.push_back(startAddress & 0xFF);        // Adres Low
    payload.push_back((quantity >> 8) & 0xFF);     // Miktar High
    payload.push_back(quantity & 0xFF);            // Miktar Low

    // Paketi birleştir
    std::vector<uint8_t> requestPacket = buildRequestPacket(unitId, Modbus::FunctionCode::ReadHoldingRegisters, payload);

    // Soket üzerinden gönder
    if (!socket.sendData(requestPacket)) {
        return {}; // Gönderme başarısız
    }

    // Cevabı bekle
    std::vector<uint8_t> response = socket.receiveData();

    // --- CEVABI DOĞRULA VE AYIKLA (PARSING) ---
    
    // 1. Boyut Kontrolü (Minimum 9 byte olmalı: 7 MBAP + 1 Func + 1 ByteCount)
    if (response.size() < 9) {
        std::cerr << "Hata: Gelen cevap cok kisa!" << std::endl;
        return {};
    }

    // 2. Transaction ID Kontrolü (Yolladığımız ID'nin bir eksiği olmalı çünkü sayacı artırmıştık)
    uint16_t expectedTxId = transactionIdCounter - 1;
    uint16_t receivedTxId = (response[0] << 8) | response[1];
    if (expectedTxId != receivedTxId) {
        std::cerr << "Hata: Transaction ID eslesmiyor!" << std::endl;
        return {};
    }

    // 3. Hata (Exception) Kontrolü
    uint8_t receivedFuncCode = response[7];
    if (receivedFuncCode == (static_cast<uint8_t>(Modbus::FunctionCode::ReadHoldingRegisters) + 0x80)) {
        std::cerr << "Modbus Cihazi Hata Dondu! Exception Code: 0x" << std::hex << (int)response[8] << std::dec << std::endl;
        return {};
    }

    // 4. Veriyi Çıkar (Register'ları oku)
    uint8_t byteCount = response[8]; // Kaç byte veri geldiğini söyler
    std::vector<uint16_t> registers;

    // Gelen veriler 9. indexten başlar. Her register 2 byte yer kaplar.
    for (size_t i = 9; i < 9 + byteCount; i += 2) {
        uint16_t value = (response[i] << 8) | response[i + 1]; // High byte ve Low byte birleştiriliyor
        registers.push_back(value);
    }

    return registers;
}

bool ModbusClient::writeSingleCoil(uint8_t unitId, uint16_t address, bool value) {
    if (!isConnected()) return false;

    std::vector<uint8_t> payload;
    
    // 1. Hedef Adres (Senin 11 numara) - Big Endian
    payload.push_back((address >> 8) & 0xFF);
    payload.push_back(address & 0xFF);
    
    // 2. Yazılacak Değer (1 ise 0xFF00, 0 ise 0x0000)
    if (value) {
        payload.push_back(0xFF); // ON
        payload.push_back(0x00);
    } else {
        payload.push_back(0x00); // OFF
        payload.push_back(0x00);
    }

    // 3. Paketi oluştur (0x05 kodu ile) ve ateşle
    std::vector<uint8_t> request = buildRequestPacket(unitId, Modbus::FunctionCode::WriteSingleCoil, payload);
    if (!socket.sendData(request)) return false;

    // 4. Cevabı dinle (Modbus, yazma işleminde gönderdiğin paketin aynısını sana geri döner)
    std::vector<uint8_t> response = socket.receiveData();

    // 5. Doğrulama (Hata kodu döndü mü?)
    if (response.size() < 12) return false; 
    if (response[7] == (static_cast<uint8_t>(Modbus::FunctionCode::WriteSingleCoil) + 0x80)) {
        std::cerr << "PLC Hata Dondu! Sesi acamadik." << std::endl;
        return false;
    }

    return true; // İşlem başarılı!
}

// 0x02 - Read Discrete Inputs
std::vector<bool> ModbusClient::readDiscreteInputs(uint8_t unitId, uint16_t startAddress, uint16_t quantity) {
    if (!isConnected()) return {};
    std::vector<uint8_t> payload = {
        static_cast<uint8_t>((startAddress >> 8) & 0xFF), static_cast<uint8_t>(startAddress & 0xFF),
        static_cast<uint8_t>((quantity >> 8) & 0xFF), static_cast<uint8_t>(quantity & 0xFF)
    };
    std::vector<uint8_t> request = buildRequestPacket(unitId, Modbus::FunctionCode::ReadDiscreteInputs, payload);
    if (!socket.sendData(request)) return {};
    std::vector<uint8_t> response = socket.receiveData();
    if (response.size() < 10 || response[7] == (static_cast<uint8_t>(Modbus::FunctionCode::ReadDiscreteInputs) + 0x80)) return {};

    std::vector<bool> bits;
    uint8_t byteCount = response[8];
    int currentBit = 0;
    for (size_t i = 0; i < byteCount; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (currentBit < quantity) {
                bits.push_back((response[9 + i] >> j) & 0x01);
                currentBit++;
            }
        }
    }
    return bits;
}

// 0x04 - Read Input Registers
std::vector<uint16_t> ModbusClient::readInputRegisters(uint8_t unitId, uint16_t startAddress, uint16_t quantity) {
    if (!isConnected()) return {};
    std::vector<uint8_t> payload = {
        static_cast<uint8_t>((startAddress >> 8) & 0xFF), static_cast<uint8_t>(startAddress & 0xFF),
        static_cast<uint8_t>((quantity >> 8) & 0xFF), static_cast<uint8_t>(quantity & 0xFF)
    };
    std::vector<uint8_t> request = buildRequestPacket(unitId, Modbus::FunctionCode::ReadInputRegisters, payload);
    if (!socket.sendData(request)) return {};
    std::vector<uint8_t> response = socket.receiveData();
    if (response.size() < 9 || response[7] == (static_cast<uint8_t>(Modbus::FunctionCode::ReadInputRegisters) + 0x80)) return {};

    uint8_t byteCount = response[8];
    std::vector<uint16_t> registers;
    for (size_t i = 9; i < 9 + byteCount; i += 2) {
        registers.push_back((response[i] << 8) | response[i + 1]);
    }
    return registers;
}

// 0x06 - Write Single Register
bool ModbusClient::writeSingleRegister(uint8_t unitId, uint16_t address, uint16_t value) {
    if (!isConnected()) return false;
    std::vector<uint8_t> payload = {
        static_cast<uint8_t>((address >> 8) & 0xFF), static_cast<uint8_t>(address & 0xFF),
        static_cast<uint8_t>((value >> 8) & 0xFF), static_cast<uint8_t>(value & 0xFF)
    };
    std::vector<uint8_t> request = buildRequestPacket(unitId, Modbus::FunctionCode::WriteSingleRegister, payload);
    if (!socket.sendData(request)) return false;
    std::vector<uint8_t> response = socket.receiveData();
    if (response.size() < 12 || response[7] == (static_cast<uint8_t>(Modbus::FunctionCode::WriteSingleRegister) + 0x80)) return false;
    return true;
}

// 0x10 - Write Multiple Registers
bool ModbusClient::writeMultipleRegisters(uint8_t unitId, uint16_t startAddress, const std::vector<uint16_t>& values) {
    if (!isConnected() || values.empty()) return false;
    uint16_t quantity = values.size();
    uint8_t byteCount = quantity * 2;
    std::vector<uint8_t> payload = {
        static_cast<uint8_t>((startAddress >> 8) & 0xFF), static_cast<uint8_t>(startAddress & 0xFF),
        static_cast<uint8_t>((quantity >> 8) & 0xFF), static_cast<uint8_t>(quantity & 0xFF),
        byteCount
    };
    for (uint16_t val : values) {
        payload.push_back((val >> 8) & 0xFF);
        payload.push_back(val & 0xFF);
    }
    std::vector<uint8_t> request = buildRequestPacket(unitId, Modbus::FunctionCode::WriteMultipleRegisters, payload);
    if (!socket.sendData(request)) return false;
    std::vector<uint8_t> response = socket.receiveData();
    if (response.size() < 12 || response[7] == (static_cast<uint8_t>(Modbus::FunctionCode::WriteMultipleRegisters) + 0x80)) return false;
    return true;
}