#ifndef MODBUS_DEFINITIONS_H
#define MODBUS_DEFINITIONS_H

#include <cstdint>
#include <iostream>
namespace Modbus {

    // Modbus standart port numarası ve başlık boyutu
    constexpr int DEFAULT_PORT = 502;
    constexpr uint16_t MBAP_HEADER_SIZE = 7;

    // Modbus Fonksiyon Kodları (Function Codes)
    enum class FunctionCode : uint8_t {
        ReadCoils               = 0x01, // Dijital çıkış (röle) okuma
        ReadDiscreteInputs      = 0x02, // Dijital giriş okuma
        ReadHoldingRegisters    = 0x03, // Analog / 16-bit yaz/oku bellek okuma
        ReadInputRegisters      = 0x04, // Sadece okunabilir 16-bit bellek okuma
        WriteSingleCoil         = 0x05, // Tek bir dijital çıkışa yazma
        WriteSingleRegister     = 0x06, // Tek bir 16-bit belleğe yazma
        WriteMultipleCoils      = 0x0F, // Birden çok dijital çıkışa yazma
        WriteMultipleRegisters  = 0x10  // Birden çok 16-bit belleğe blok halinde yazma
    };

    // Modbus Hata Kodları (Exception Codes)
    // Server (Slave) bir isteği yerine getiremezse bu kodlardan birini döner
    enum class ExceptionCode : uint8_t {
        None                                = 0x00, // Hata yok (bizim kontrolümüz için)
        IllegalFunction                     = 0x01, // Cihaz bu fonksiyon kodunu desteklemiyor
        IllegalDataAddress                  = 0x02, // İstenen bellek adresi cihazda yok
        IllegalDataValue                    = 0x03, // Gönderilen veri değeri cihaz için geçersiz
        ServerDeviceFailure                 = 0x04, // Cihaz isteği işlerken kurtarılamaz bir hata oluştu
        Acknowledge                         = 0x05, // Cihaz isteği aldı ama işlem uzun sürecek
        ServerDeviceBusy                    = 0x06, // Cihaz şu an meşgul, isteği işleyemiyor
        MemoryParityError                   = 0x08, // Bellek eşlik hatası
        GatewayPathUnavailable              = 0x0A, // Gateway (ağ geçidi) üzerinden cihaza ulaşılamıyor
        GatewayTargetDeviceFailedToRespond  = 0x0B  // Hedef cihaz Gateway'e cevap vermedi
    };

} // namespace Modbus

#endif // MODBUS_DEFINITIONS_H