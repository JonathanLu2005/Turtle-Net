#ifdef _WIN32 
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

#include <iostream>
#include <string>
#include <sys/types.h>
#include <fstream>
#define SFML_STATIC
#include <SFML/Graphics.hpp>

constexpr unsigned int WindowWidth = 800;
constexpr unsigned int WindowHeight = 600;

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

struct DrawnLines {
    sf::Vector2f LineStart;
    sf::Vector2f LineEnd;
};


void SendCode(int Sock, const std::string& LogoCode) {
    uint32_t Length = htonl(LogoCode.size());

    if (!SendAll(Sock, (const char*)&Length, sizeof(Length))) {
        return;
    }

    if (!SendAll(Sock, LogoCode.data(), LogoCode.size())) {
        return;
    }
}

std::vector<DrawnLines> ReceiveLinesFromServer(int Sock) {
    uint32_t Count;

    if (!ReceiveAll(Sock, (char*)&Count, sizeof(Count))) {
        return {};
    }

    Count = ntohl(Count);
    std::vector<DrawnLines> Lines(Count);

    for (uint32_t i = 0; i < Count; ++i) {
        float Coords[4];

        if (!ReceiveAll(Sock, (char*)Coords, sizeof(Coords))) {
            return {};
        }

        Lines[i].LineStart.x = Coords[0];
        Lines[i].LineStart.y = Coords[1];
        Lines[i].LineEnd.x = Coords[2];
        Lines[i].LineEnd.y = Coords[3];
    }
    return Lines;
}

int main() {
#ifdef _WIN32 
    WSADATA WSAData;
    if (WSAStartup(MAKEWORD(2,2), &WSAData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }
#endif

    int Sock = 
#ifdef _WIN32
    socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#else 
    socket(AF_INET, SOCK_STREAM, 0);
#endif



    sockaddr_in ServerAddress{};
    ServerAddress.sin_family = AF_INET;
    ServerAddress.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &ServerAddress.sin_addr);

    if (connect(Sock, (struct sockaddr*)&ServerAddress, sizeof(ServerAddress)) < 0) {
        std::cerr << "Connection failed\n";
    #ifdef _WIN32 
        closesocket(Sock);
        WSACleanup();
    #else
        close(Sock);
    #endif
        return 1;
    }

    // Receive file path
    std::string LogoPath;
    std::cout << "Enter the directory for the logo code: ";
    std::getline(std::cin, LogoPath);

    // If file path exists
    if (LogoPath.empty()) {
        std::cerr << "Error: No file path provided" << std::endl;
        return 1;
    }

    // Access file
    std::ifstream LogoFile(LogoPath);
    std::string LogoCode((std::istreambuf_iterator<char>(LogoFile)), std::istreambuf_iterator<char>());
    
    // If file exists
    if (!LogoFile) {
        std::cerr << "Error: Unable to open file" << std::endl;
        return 1;
    }

    SendCode(Sock, LogoCode);

    auto Lines = ReceiveLinesFromServer(Sock);

    if (Lines.size() == 0) {
        std::cerr << "Fail\n";
        return 1;
    }
    
    sf::RenderWindow Window(sf::VideoMode({static_cast<unsigned int>(WindowWidth),static_cast<unsigned int>(WindowHeight)}), "TurtleNet Image");
    while (Window.isOpen()) {
        while (const std::optional<sf::Event> event = Window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                Window.close();
            }
        }

        Window.clear();

        for (const auto& Line : Lines) {
            sf::Vertex Vertices[2];
            Vertices[0].position = Line.LineStart;
            Vertices[1].position = Line.LineEnd;
            Window.draw(Vertices, 2, sf::PrimitiveType::Lines);
        }

        Window.display();
    }    

#ifdef _WIN32 
    closesocket(Sock);
    WSACleanup();
#else
    close(Sock);
#endif

    return 0;
}