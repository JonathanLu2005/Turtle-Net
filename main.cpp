// Libraries
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <memory>
#include <boost/spirit/include/qi.hpp>
#include <boost/phoenix.hpp>

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

    void Origin() {
        X = 0;
        Y = 0;
    }

    void PenState(bool CurrentPen) {
        PenDown = CurrentPen;
    }

    void MovePosition(float Distance) {
        X += Distance * cos((Distance * 3.14)/180);
        Y += Distance * sin((Distance * 3.14)/180);
    }

    void Turn(float Angle) {
        Direction += Angle;

        if (Direction < 0) { 
            Direction += 360;
        } 
        Direction = std::fmod(Direction, 360.0f);
    }
};

// AST nodes
// Abstract node as template
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void Execute(TurtleState& Turtle) = 0;
    virtual std::string ToString() const = 0;
};

// Movement node
class MovementNode : public ASTNode {
    float Steps;

public:
    MovementNode(float Steps) : Steps(Steps) {}

    float GetSteps() const { return Steps; }

    void Execute(TurtleState& Turtle) override {
        Turtle.MovePosition(Steps);
    }

    std::string ToString() const override {
        return "MovementNode: " + std::to_string(Steps);
    }
};

// Direction node
class DirectionNode : public ASTNode {
    float Angle;

public:
    DirectionNode(float Angle) : Angle(Angle) {}

    float GetAngle() const { return Angle; }

    void Execute(TurtleState& Turtle) override {
        Turtle.Turn(Angle);
    }

    std::string ToString() const override {
        return "DirectionNode: " + std::to_string(Angle);
    }
};

// Pen node
class PenNode : public ASTNode {
    bool PenInUse;

public:
    PenNode(bool PenInUse) : PenInUse(PenInUse) {}

    bool GetPenStatus() const { return PenInUse; }

    void Execute(TurtleState& Turtle) override {
        Turtle.PenState(PenInUse);
    }

    std::string ToString() const override {
        return "PenNode: " + std::string(PenInUse ? "Down" : "Up");
    }
};

// Loop node
class LoopNode : public ASTNode {
    int Iterations;
    std::vector<std::shared_ptr<ASTNode>> Commands;

public:
    LoopNode(int Iterations) : Iterations(Iterations) {}

    void AddCommand(std::shared_ptr<ASTNode> NewCommand) {
        Commands.push_back(std::move(NewCommand));
    }

    int GetIterations() const { return Iterations; }
    const std::vector<std::shared_ptr<ASTNode>>& GetCommands() const { return Commands; }

    void Execute(TurtleState& Turtle) override {
        for (int i = 0; i < Iterations; i++) {
            for (auto& Command : Commands) {
                Command->Execute(Turtle);
            }
        }
    }

    std::string ToString() const override {
        std::string Format = "IterationNodes: " + std::to_string(Iterations) + " [";

        for (const auto& Command : Commands) {
            Format += "\n  " + Command->ToString();
        }
        Format += "\n]";

        return Format;
    }
};

// Origin node
class OriginNode : public ASTNode {
    bool ClearScreen;

public:
    OriginNode(bool ClearScreen) : ClearScreen(ClearScreen) {}

    bool GetClearScreen() const { return ClearScreen; }

    void Execute(TurtleState& Turtle) override {}

    std::string ToString() const override {
        return "OriginNode: " + std::string(ClearScreen ? "Clear Screen" : "Home");
    }
};

// Comment node
class CommentNode : public ASTNode {
    std::string Comment;

public:
    CommentNode(const std::string& Comment) : Comment(Comment) {}

    std::string GetComment() const { return Comment; }

    void Execute(TurtleState& Turtle) override {}

    std::string ToString() const override {
        return "CommentNode: " + Comment;
    }
};


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
    // Creates AST Tree
    std::vector<std::shared_ptr<ASTNode>> ASTTree;

    using Iterator = std::string::const_iterator;

    // Grammars
    qi::rule<Iterator, std::string(), ascii::space_type> Movement;
    qi::rule<Iterator, std::string(), ascii::space_type> Direction;
    qi::rule<Iterator, std::string(), ascii::space_type> Pen;
    qi::rule<Iterator, std::string(), ascii::space_type> Origin;
    qi::rule<Iterator, float(), ascii::space_type> Value;
    qi::rule<Iterator, std::string(), ascii::space_type> Comment;

    qi::rule<Iterator, std::shared_ptr<ASTNode>(), ascii::space_type> MovementCommand;
    qi::rule<Iterator, std::shared_ptr<ASTNode>(), ascii::space_type> DirectionCommand;
    qi::rule<Iterator, std::shared_ptr<ASTNode>(), ascii::space_type> PenCommand;
    qi::rule<Iterator, std::shared_ptr<ASTNode>(), ascii::space_type> OriginCommand;
    qi::rule<Iterator, std::shared_ptr<ASTNode>(), ascii::space_type> LoopCommand;
    qi::rule<Iterator, std::shared_ptr<ASTNode>(), ascii::space_type> CommentCommand;
    qi::rule<Iterator, std::shared_ptr<ASTNode>(), ascii::space_type> Command;
    qi::rule<Iterator, std::vector<std::shared_ptr<ASTNode>>(), ascii::space_type> Commands; 
    qi::rule<Iterator, std::vector<std::shared_ptr<ASTNode>>(), ascii::space_type> Program;

    LogoParser () {
        Movement = qi::string("forward") | qi::string("back");
        Direction = qi::string("left") | qi::string("right");
        Pen = qi::string("penup") | qi::string("pendown");
        Origin = qi::string("clearscreen") | qi::string("home");
        Value = qi::float_;
        Comment = qi::lexeme[';' >> *(qi::char_ - '\n')];
        
        MovementCommand = (Movement >> Value) [
            qi::_val = boost::phoenix::construct<std::shared_ptr<ASTNode>>(
                boost::phoenix::new_<MovementNode>(
                    boost::phoenix::if_else(qi::_1 == "forward", qi::_2, -qi::_2)
                )
            )
        ];

        DirectionCommand = (Direction >> Value) [
            qi::_val = boost::phoenix::construct<std::shared_ptr<ASTNode>>(
                boost::phoenix::new_<DirectionNode>(
                    boost::phoenix::if_else(qi::_1 == "right", qi::_2, -qi::_2)
                )
            )
        ];

        PenCommand = (Pen) [
            qi::_val = boost::phoenix::construct<std::shared_ptr<ASTNode>>(
                boost::phoenix::new_<PenNode>(
                    boost::phoenix::if_else(qi::_1 == "pendown", true, false)
                )
            )
        ];

        OriginCommand = (Origin) [
            qi::_val = boost::phoenix::construct<std::shared_ptr<ASTNode>>(
                boost::phoenix::new_<OriginNode>(
                    boost::phoenix::if_else(qi::_1 == "clearscreen", true, false)
                )
            )
        ];

        LoopCommand = (qi::lit("repeat") >> Value >> qi::lit('[') >> +Command >> qi::lit(']')) [
            boost::phoenix::bind(
                [](auto& val, float Iterations, std::vector<std::shared_ptr<ASTNode>>& Commands) {
                    auto Loop = std::make_shared<LoopNode>(static_cast<int>(Iterations));

                    for (auto& Command : Commands) {
                        Loop->AddCommand(Command);
                    }

                    val = Loop;
                },
                qi::_val, qi::_1, qi::_2
            )
        ];

        CommentCommand = (Comment) [
            qi::_val = boost::phoenix::construct<std::shared_ptr<ASTNode>>(
                boost::phoenix::new_<CommentNode>(qi::_1)
            )
        ];
        
        Command = MovementCommand | DirectionCommand | PenCommand | OriginCommand | LoopCommand | CommentCommand;
        Program = +Command;
    }

    // Returns AST Tree
    const std::vector<std::shared_ptr<ASTNode>>& GetAST() const {
        return ASTTree;
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
    std::vector<std::shared_ptr<ASTNode>> AST;
    // Parse
    bool ParseResult = qi::phrase_parse(First,Last,Parser.Program >> qi::eoi, ascii::space, AST);

    if (ParseResult && First == Last) {
        Parser.ASTTree = std::move(AST);
        std::cout << "Success" << std::endl;
        std::cout << "AST size: " << AST.size() << std::endl;

        for (const auto& Node : Parser.GetAST()) {
            if (Node) {
                std::cout << Node->ToString() << std::endl;
            }
        }
    } else {
        std::cerr << "Fail" << std::endl;
    }

    return 0;
}