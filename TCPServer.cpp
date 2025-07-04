#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

#include "ThreadPoolServer.h"
#include "LogoEngine.hpp"
#include <iostream>
#include <vector>

#define PORT 12345

bool SendAll(int Sock, const char* Data, size_t Length) {
    size_t TotalSent = 0;
    std::cout << "[SendAll] Want to send " << Length << " bytes" << std::endl;
    while (TotalSent < Length) {
        int Sent = send(Sock, Data + TotalSent, static_cast<int>(Length - TotalSent), 0);
        if (Sent <= 0) {
            std::cerr << "[SendAll] Error or connection closed after sending " << TotalSent << " bytes" << std::endl;
            return false;
        }
        std::cout << "[SendAll] Sent " << Sent << " bytes (Total: " << (TotalSent + Sent) << "/" << Length << ")" << std::endl;
        TotalSent += Sent;
    }
    std::cout << "[SendAll] Finished sending " << TotalSent << " bytes" << std::endl;
    return true;
}

bool ReceiveAll(int Sock, char* Data, size_t Length) {
    size_t TotalReceived = 0;
    std::cout << "[ReceiveAll] Want to receive " << Length << " bytes" << std::endl;
    while (TotalReceived < Length) {
        int Received = recv(Sock, Data + TotalReceived, static_cast<int>(Length - TotalReceived), 0);
        if (Received <= 0) {
            std::cerr << "[ReceiveAll] Error or connection closed after receiving " << TotalReceived << " bytes" << std::endl;
            return false;
        }
        std::cout << "[ReceiveAll] Received " << Received << " bytes (Total: " << (TotalReceived + Received) << "/" << Length << ")" << std::endl;
        TotalReceived += Received;
    }
    std::cout << "[ReceiveAll] Finished receiving " << TotalReceived << " bytes" << std::endl;
    return true;
}

std::string ReceiveCode(int ClientSock) {
    uint32_t Length;

    if (!ReceiveAll(ClientSock, (char*)&Length, sizeof(Length))) {
        return "";
    }

    Length = ntohl(Length);
    std::string LogoCode(Length, '\0');

    if (!ReceiveAll(ClientSock, &LogoCode[0], Length)) {
        return "";
    }

    return LogoCode;
}

void SendLines(int ClientSock, const std::vector<DrawnLines>& Lines) {
    uint32_t Count = htonl(static_cast<uint32_t>(Lines.size()));

    if (!SendAll(ClientSock, (const char*)&Count, sizeof(Count))) {
        return;
    }

    for (const auto& Line : Lines) {
        float Coords[4] = {Line.LineStart.x, Line.LineStart.y, Line.LineEnd.x, Line.LineEnd.y};

        if (!SendAll(ClientSock, (const char*)Coords, sizeof(Coords))) {
            return;
        }
    }
}

int main() {
#ifdef _WIN32
    WSADATA WSAData;
    int WSAInit = WSAStartup(MAKEWORD(2,2), &WSAData);
    if (WSAInit != 0) {
        std::cerr << "WSAStartup failed with error: " << WSAInit << std::endl;
        return 1;
    }
#endif

    int ServerFD = socket(AF_INET, SOCK_STREAM, 0);

    if (ServerFD < 0) {
        std::cerr << "Failed to create socket \n";
        return 1;
    }

    sockaddr_in Address{};
    Address.sin_family = AF_INET;
    Address.sin_addr.s_addr = INADDR_ANY;
    Address.sin_port = htons(PORT);

    if (bind(ServerFD, (struct sockaddr*)&Address, sizeof(Address)) < 0) {
        std::cerr << "Bind failed\n";
    #ifdef _WIN32 
        closesocket(ServerFD);
    #else 
        close(ServerFD);
    #endif
        return 1;
    }

    if (listen(ServerFD, 10) < 0) {
        std::cerr << "Listen failed\n";
    #ifdef _WIN32 
        closesocket(ServerFD);
    #else 
        close(ServerFD);
    #endif
        return 1;
    }

    //bind(ServerFD, (struct sockaddr*)&Address, sizeof(Address));
    //listen(ServerFD, 3);

    ThreadPool Pool(4);

    while (true) {
        sockaddr_in ClientAddress;
        socklen_t ClientLength = sizeof(ClientAddress);

        int ClientSock = accept(ServerFD, (struct sockaddr*)&ClientAddress, &ClientLength);

        if (ClientSock < 0) {
            std::cerr << "Failed to connect\n";
            return 1;
        }

        Pool.Enqueue([ClientSock] {
            std::string LogoCode = ReceiveCode(ClientSock);
            LogoEngine Engine;
            auto Lines = Engine.Run(LogoCode);
            std::cout << "Engine produced " << Lines.size() << " lines" << std::endl;
            SendLines(ClientSock, Lines);
        #ifdef _WIN32 
            closesocket(ClientSock);
        #else 
            close(ClientSock);
        #endif
        });
    }

#ifdef _WIN32 
    closesocket(ServerFD);
#else 
    close(ServerFD);
#endif
    return 0;
}