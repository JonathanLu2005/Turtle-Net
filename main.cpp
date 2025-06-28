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
    WHITESPACE,
    OPEN,
    CLOSE
};

// Parser
class LogoParser {
public:
    using Iterator = std::string::const_iterator;

    // Grammars
    qi::rule<Iterator, std::string(), ascii::space_type> Movement;
    qi::rule<Iterator, std::string(), ascii::space_type> Direction;
    qi::rule<Iterator, std::string(), ascii::space_type> Pen;
    qi::rule<Iterator, std::string(), ascii::space_type> Origin;
    qi::rule<Iterator, float(), ascii::space_type> Value;
    qi::rule<Iterator, std::string(), ascii::space_type> Comment;

    qi::rule<Iterator, ascii::space_type> MovementCommand;
    qi::rule<Iterator, ascii::space_type> DirectionCommand;
    qi::rule<Iterator, ascii::space_type> PenCommand;
    qi::rule<Iterator, ascii::space_type> OriginCommand;
    qi::rule<Iterator, ascii::space_type> LoopCommand;
    qi::rule<Iterator, ascii::space_type> CommentCommand;
    qi::rule<Iterator, ascii::space_type> Command;
    qi::rule<Iterator, ascii::space_type> Program;

    LogoParser () {
        Movement = qi::lit("forward") | "back";
        Direction = qi::lit("left") | "right";
        Pen = qi::lit("penup") | "pendown";
        Origin = qi::lit("clearscreen") | "home";
        Value = qi::float_;
        Comment = qi::lexeme[';' >> *(qi::char_ - '\n')];


        MovementCommand = Movement >> Value;
        DirectionCommand = Direction >> Value;
        PenCommand = Pen;
        OriginCommand = Origin;
        LoopCommand = qi::lit("repeat") >> Value >> qi::lit('[') >> +Command >> qi::lit(']');
        CommentCommand = Comment;
        Command = MovementCommand | DirectionCommand | PenCommand | OriginCommand | LoopCommand | CommentCommand;
        Program = +Command;
    }
};

class TurtleState {
public:
    // Attributes
    float X;
    float Y;
    float Direction;
    bool PenDown;

    // Instantiator
    TurtleState(float StartingX = 0, float StartingY = 0, float StartingDirection = 0, bool StartingPenDown = true)
    : X(StartingX), Y(StartingY), Direction(StartingDirection), PenDown(StartingPenDown) {}

    void PenUp() {
        PenDown = true;
    }

    void PenDown() {
        PenDown = false;
    }

    void MovePosition(float Distance) {
        X = Distance * cos(Distance * 3.14/180);
        Y = Distance * sin(Distance * 3.14/180);
    }

    void TurnLeft(float Angle) {
        Direction -= Angle;

        if (Direction < 0) {
            Direction += 360;
        }
    }

    void TurnRight(float Angle) {
        Direction = (Direction + Angle) % 360;
    }
}

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
    TurtleState MyTurtle(0,0,0,true);

    // Instantiate parser
    LogoParser Parser;
    LogoParser::Iterator First = LogoCode.begin();
    LogoParser::Iterator Last = LogoCode.end();
    // Parse
    bool ParseResult = qi::phrase_parse(First,Last,Parser.Program >> qi::eoi, ascii::space);

    if (ParseResult && First == Last) {
        std::cout << "Success" << std::endl;
    } else {
        std::cerr << "Fail" << std::endl;
    }

    return 0;
}