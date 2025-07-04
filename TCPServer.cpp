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

std::string ReceiveCode(int ClientSock) {
    uint32_t Length;
    recv(ClientSock, (char*)&Length, sizeof(Length), 0);
    Length = ntohl(Length);
    std::string LogoCode(Length, '\0');
    recv(ClientSock, &LogoCode[0], Length, 0);
    return LogoCode;
}

void SendLines(int ClientSock, const std::vector<DrawnLines>& Lines) {
    uint32_t Count = htonl(static_cast<uint32_t>(Lines.size()));
    send(ClientSock, (const char*)&Count, sizeof(Count), 0);

    for (const auto& Line : Lines) {
        float Coords[4] = {Line.LineStart.x, Line.LineStart.y, Line.LineEnd.x, Line.LineEnd.y};
        send(ClientSock, (const char*)&Coords, sizeof(Coords), 0);
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