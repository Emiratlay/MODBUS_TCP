#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
// Windows Soket Kütüphaneleri
#include <winsock2.h>
#include <ws2tcpip.h>

class TcpSocket {
public:
    TcpSocket();
    ~TcpSocket();

    // Temel Ağ Fonksiyonları
    bool connectTo(const std::string& ipAddress, int port);
    void disconnect();
    bool sendData(const std::vector<uint8_t>& data);
    std::vector<uint8_t> receiveData(int bufferSize = 1024);

    bool getIsConnected() const { return isConnected; }

private:
    SOCKET sock;
    bool isConnected;
};

#endif // TCP_SOCKET_H