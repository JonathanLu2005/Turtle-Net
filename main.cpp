// Libraries
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "Turtle.hpp"
#include "AST.hpp"
#include "LogoParser.hpp"
#include <memory>
#include <boost/spirit/include/qi.hpp>
#include <boost/phoenix.hpp>
#define SFML_STATIC
#include <SFML/Graphics.hpp>

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

std::vector<DrawnLines> Lines;

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

    // Instantiate Turtle State
    TurtleState MyTurtle(WindowWidth / 2.0f, WindowHeight / 2.0f, WindowWidth / 2.0f, WindowHeight / 2.0f,0,true);
    MyTurtle.ImageLines = &Lines;

    // Instantiate parser
    LogoParser Parser;
    LogoParser::Iterator First = LogoCode.begin();
    LogoParser::Iterator Last = LogoCode.end();
    std::vector<std::shared_ptr<ASTNode>> AST;
    // Parse
    bool ParseResult = qi::phrase_parse(First,Last,Parser.Program >> qi::eoi, ascii::space, AST);

    if (ParseResult && First == Last) {
        Parser.ASTTree = std::move(AST);

        for (const auto& Node : Parser.GetAST()) {
            if (Node) {
                Node->Execute(MyTurtle);
                //std::cout << Node->ToString() << std::endl;
            }
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

    } else {
        std::cerr << "Fail" << std::endl;
    }

    return 0;
}