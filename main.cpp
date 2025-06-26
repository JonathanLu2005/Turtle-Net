// Libraries
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <boost/spirit/include/qi.hpp>
#include <boost/phoenix.hpp>

// Namespaces
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

// Token types
enum TokenType {
    MOVEMENT,
    DIRECTION,
    PEN,
    LOOP,
    ORIGIN,
    VALUE,
    COMMENT,
    PLAINTEXT,
    WHITESPACE
};

// Parser
class LogoParser {
public:
    using Iterator = std::string::const_iterator;

    // Lexer
    qi::rule<std::string::iterator, std::string()> Movement = qi::lit("forward") | "back";
    qi::rule<std::string::iterator, std::string()> Direction = qi::lit("left") | "right";
    qi::rule<std::string::iterator, std::string()> Pen = qi::lit("penup") | "pendown";
    qi::rule<std::string::iterator, std::string()> Origin = qi::lit("clearscreen") | "home";
    qi::rule<std::string::iterator, float()> Value = qi::float_;
    qi::rule<std::string::iterator, std::string()> Comment = qi::lit(';') >> *(qi::char_);
    qi::rule<std::string::iterator, std::string()> Whitespace = +qi::space;
    qi::rule<std::string::iterator, std::string()> Loop = qi::lit("repeat");

    // Grammars
    qi::rule<std::string::iterator> MovementCommand;
    qi::rule<std::string::iterator> DirectionCommand;
    qi::rule<std::string::iterator> PenCommand;
    qi::rule<std::string::iterator> OriginCommand;
    qi::rule<std::string::iterator> LoopCommand;
    qi::rule<std::string::iterator> CommentCommand;
    qi::rule<std::string::iterator> Program;

    LogoParser () {
        MovementCommand = Movement >> Whitespace >> Value >> Whitespace;
        DirectionCommand = Direction >> Whitespace >> Value >> Whitespace;
        PenCommand = Pen >> Whitespace;
        OriginCommand = Origin >> Whitespace;
        LoopCommand = Loop >> Whitespace >> Value >> Whitespace >> '[' >> *(Program) >> ']' >> Whitespace;
        CommentCommand = Comment >> Whitespace;
        Program = *(MovementCommand | DirectionCommand | PenCommand | OriginCommand | LoopCommand | CommentCommand);
    }
};

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
    std::ifstream LogoCode(LogoPath);
    
    // If file exists
    if (!LogoCode) {
        std::cerr << "Error: Unable to open file" << std::endl;
        return 1;
    }

    return 0;
}