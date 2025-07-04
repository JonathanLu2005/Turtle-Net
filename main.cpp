// Libraries
#include <iostream>
#include <fstream>
#include "LogoEngine.hpp"
#define SFML_STATIC
#include <SFML/Graphics.hpp>

// Main
int main() {
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

    LogoEngine Engine;
    auto Lines = Engine.Run(LogoCode);



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


    return 0;
}