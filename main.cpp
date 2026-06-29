#include <iostream>
#include "ModbusClient.h"
#include <windows.h> // Sleep fonksiyonu için
//g++ *.cpp -o main.exe -lws2_32
int main() {
    ModbusClient modbus;
    
    std::string plcIp = "192.168.1.100";
    int plcPort = 502;
    uint8_t unitId = 1;         // Slave ID genelde 1'dir
    uint16_t address = 10;      // Senin MB0 bitinin adresi
    
    std::cout << plcIp << " adresine baglaniliyor..." << std::endl;
    
    if (modbus.connect(plcIp, plcPort)) {
        std::cout << "Baglanti Kuruldu! Sesi tetikliyoruz..." << std::endl;

        // Adres 11'e TRUE (1) yaz (Ses başlasın)
        if (modbus.writeSingleCoil(unitId, address, true)) {
            std::cout << "--> Bip sesi basladi! (QP Aktif)" << std::endl;
        } else {
            std::cout << "Hata: Yazi yazilamadi!" << std::endl;
        }

        // 2 saniye bekle
        Sleep(2000); 

        // Adres 11'e FALSE (0) yaz (Ses sussun)
        if (modbus.writeSingleCoil(unitId, address, false)) {
            std::cout << "--> Bip sesi sustu! (QP Kapali)" << std::endl;
        }

        modbus.disconnect();
    } else {
        std::cout << "Baglanti hatasi!" << std::endl;
    }

    return 0;
}