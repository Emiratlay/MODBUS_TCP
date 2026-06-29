#include "TcpSocket.h"

// Derleyiciye Winsock kütüphanesini baglamasini soyluyoruz (MSVC icin)
#pragma comment(lib, "Ws2_32.lib")

TcpSocket::TcpSocket() : sock(INVALID_SOCKET), isConnected(false) {
    // 1. Adim: Winsock kutuphanesini baslat (Sadece Windows'ta gereklidir)
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup basarisiz oldu!" << std::endl;
    }
}

TcpSocket::~TcpSocket() {
    disconnect();
    // Winsock kaynaklarini serbest birak
    WSACleanup();
}

bool TcpSocket::connectTo(const std::string& ipAddress, int port) {
    if (isConnected) disconnect();

    // 2. Adim: Soket olustur (IPv4, TCP)
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Soket olusturulamadi. Hata kodu: " << WSAGetLastError() << std::endl;
        return false;
    }

    // 3. Adim: Baglanilacak adres yapisini hazirla
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port); // Port numarasini Big-Endian'a cevir

    // IP adresini string'den makine formatina cevir
    if (inet_pton(AF_INET, ipAddress.c_str(), &serverAddress.sin_addr) <= 0) {
        std::cerr << "Gecersiz IP adresi!" << std::endl;
        closesocket(sock);
        return false;
    }

    // 4. Adim: Baglantiyi kur
    if (connect(sock, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Baglanti basarisiz! Hata kodu: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        sock = INVALID_SOCKET;
        return false;
    }

    isConnected = true;
    return true;
}

void TcpSocket::disconnect() {
    if (sock != INVALID_SOCKET) {
        closesocket(sock);
        sock = INVALID_SOCKET;
    }
    isConnected = false;
}

bool TcpSocket::sendData(const std::vector<uint8_t>& data) {
    if (!isConnected || data.empty()) return false;

    // Veriyi ethernet uzerinden gonder
    int bytesSent = send(sock, reinterpret_cast<const char*>(data.data()), data.size(), 0);
    
    if (bytesSent == SOCKET_ERROR) {
        std::cerr << "Veri gonderilemedi! Hata kodu: " << WSAGetLastError() << std::endl;
        disconnect();
        return false;
    }
    return true;
}

std::vector<uint8_t> TcpSocket::receiveData(int bufferSize) {
    if (!isConnected) return {};

    std::vector<uint8_t> buffer(bufferSize);
    
    // Veriyi bekle ve oku
    int bytesReceived = recv(sock, reinterpret_cast<char*>(buffer.data()), bufferSize, 0);
    
    if (bytesReceived > 0) {
        buffer.resize(bytesReceived); // Buffer'i sadece gelen veri boyutuna kucult
        return buffer;
    } else if (bytesReceived == 0) {
        std::cerr << "Karsi taraf baglantiyi kapatti." << std::endl;
        disconnect();
    } else {
        std::cerr << "Veri okuma hatasi! Hata kodu: " << WSAGetLastError() << std::endl;
        disconnect();
    }
    
    return {};
}

