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
#include <immintrin.h>
#include <vector>

constexpr unsigned int WindowWidth = 800;
constexpr unsigned int WindowHeight = 600;

#define PORT 12345

struct DrawnLines {
    sf::Vector2f LineStart;
    sf::Vector2f LineEnd;
};


void SendCode(int Sock, const std::string& LogoCode) {
    uint32_t Length = htonl(static_cast<uint32_t>(LogoCode.size()));
    send(Sock, (const char*)&Length, sizeof(Length),0);
    send(Sock, LogoCode.data(), LogoCode.size(), 0);
}

std::vector<DrawnLines> ReceiveLinesFromServer(int Sock) {
    uint32_t Count;
    recv(Sock, (char*)&Count, sizeof(Count), 0);

    Count = ntohl(Count);
    std::vector<DrawnLines> Lines(Count);

    std::vector<float> Coords(Count * 4);
    size_t TotalBytes = Count * 4 * sizeof(float);
    size_t BytesReceived = 0;
    char* Buffer = (char*)Coords.data();
    while (BytesReceived < TotalBytes) {
        int Retrieve = recv(Sock, Buffer + BytesReceived, TotalBytes - BytesReceived, 0);
        if (Retrieve <= 0) {
            break;
        }
        BytesReceived += Retrieve;
    }

    size_t i = 0;

    for (; i + 3 < Count; i += 4) {
        __m128 X0 = _mm_loadu_ps(&Coords[(i+0) * 4]);
        __m128 X1 = _mm_loadu_ps(&Coords[(i+1) * 4]);
        __m128 X2 = _mm_loadu_ps(&Coords[(i+2) * 4]);
        __m128 X3 = _mm_loadu_ps(&Coords[(i+3) * 4]);

        float Temp[4];
        _mm_storeu_ps(Temp, X0);
        Lines[i+0].LineStart.x = Temp[0];
        Lines[i+0].LineStart.y = Temp[1];
        Lines[i+0].LineEnd.x = Temp[2];
        Lines[i+0].LineEnd.y = Temp[3];

        _mm_storeu_ps(Temp, X1);
        Lines[i+1].LineStart.x = Temp[0];
        Lines[i+1].LineStart.y = Temp[1];
        Lines[i+1].LineEnd.x = Temp[2];
        Lines[i+1].LineEnd.y = Temp[3];

        _mm_storeu_ps(Temp, X2);
        Lines[i+2].LineStart.x = Temp[0];
        Lines[i+2].LineStart.y = Temp[1];
        Lines[i+2].LineEnd.x = Temp[2];
        Lines[i+2].LineEnd.y = Temp[3];

        _mm_storeu_ps(Temp, X3);
        Lines[i+3].LineStart.x = Temp[0];
        Lines[i+3].LineStart.y = Temp[1];
        Lines[i+3].LineEnd.x = Temp[2];
        Lines[i+3].LineEnd.y = Temp[3];
    }

    for (; i < Count; ++i) {
        Lines[i].LineStart.x = Coords[(i*4) + 0];
        Lines[i].LineStart.y = Coords[(i*4) + 1];
        Lines[i].LineEnd.x = Coords[(i*4) + 2];
        Lines[i].LineEnd.y = Coords[(i*4) + 3];
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