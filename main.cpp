// Libraries
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <memory>
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
    // Creates AST Tree
    std::vector<std::unique_ptr<ASTNode>> ASTTree;

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

        MovementCommand = (Movement >> Value)[
            [&](auto& ctx) {
                std::string MoveType = qi::_1(ctx);
                float Steps = qi::_2(ctx);

                if (MoveType == "forward") {
                    ASTTree.push_back(std::make_unique<MovementNode>(Steps));
                } else {
                    ASTTree.push_back(std::make_unique<MovementNode>(-Steps));
                }
            }
        ];

        DirectionCommand = (Direction >> Value)[
            [&](auto& ctx) {
                std::string DirectionType = qi::_1(ctx);
                float Angle = qi::_2(ctx);

                if (DirectionType == "right") {
                    ASTTree.push_back(std::make_unique<DirectionNode>(Angle));
                } else {
                    ASTTree.push_back(std::make_unique<DirectionNode>(-Angle));
                }
            }
        ];

        PenCommand = (Pen)[
            [&](auto& ctx) {
                std::string PenState = qi::_1(ctx);

                if (PenState == "pendown") {
                    ASTTree.push_back(std::make_unique<PenNode>(true));
                } else {
                    ASTTree.push_back(std::make_unique<PenNode>(false));
                }
            }
        ];

        OriginCommand = (Origin)[
            [&](auto& ctx) {
                std::string OriginState = qi::_1(ctx);

                if (OriginState == "clearscreen") {
                    ASTTree.push_back(std::make_unique<OriginNode>(true, &ASTTree));
                } else {
                    ASTTree.push_back(std::make_unique<OriginNode>(false, &ASTTree));
                }
            }
        ];

        LoopCommand = (qi::lit("repeat") >> Value >> qi::lit('[') >> +Command >> qi::lit(']'))[
            [&](auto& ctx) {
                float Iterations = qi::_1(ctx);
                auto LoopNode = std::make_unique<LoopNode>(Iterations);

                for (const auto& Command : qi::_2(ctx)) {
                    LoopNode->AddCommand(std::move(Command));
                }

                ASTTree.push_back(std::move(LoopNode));
            }
        ];

        CommentCommand = Comment;
        Command = MovementCommand | DirectionCommand | PenCommand | OriginCommand | LoopCommand | CommentCommand;
        Program = +Command;
    }

    // Returns AST Tree
    const std::vector<std::unique_ptr<ASTNode>>& GetAST() const {
        return ASTTree;
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

    void Origin() {
        X = 0;
        Y = 0;
    }

    void PenState(bool CurrentPen) {
        PenDown = CurrentPen;
    }

    void MovePosition(float Distance) {
        X = Distance * cos(Distance * 3.14/180);
        Y = Distance * sin(Distance * 3.14/180);
    }

    void Turn(float Angle) {
        Direction += Angle;

        if (Direction < 0) { 
            Direction += 360;
        } 
        Direction = Direction % 360;
    }
}

// AST nodes
// Abstract node as template
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void Execute() = 0;
};

// Movement node
class MovementNode : public ASTNode {
    float Steps;

public:
    MovementNode(float Steps) : Steps(Steps) {}

    float GetSteps() const { return Steps; }

    void Execute() override {
        MovePosition(Steps);
    }
};

// Direction node
class DirectionNode : public ASTNode {
    float Angle;

public:
    DirectionNode(float Angle) : Angle(Angle) {}

    float GetAngle() const { return Angle; }

    void Execute() override {
        Turn(Angle);
    }
};

// Pen node
class PenNode : public ASTNode {
    bool PenInUse;

public:
    PenNode(bool PenInUse) : PenInUse(PenInUse) {}

    bool GetPenStatus() const { return PenInUse; }

    void Execute() override {
        PenState(PenInUse);
    }
};

// Loop node
class LoopNode : public ASTNode {
    int Iterations;
    std::vector<std::unique_ptr<ASTNode>> Commands;

public:
    LoopNode(int Iterations) : Iterations(Iterations) {}

    void AddCommand(std::unique_ptr<ASTNode> NewCommand) {
        Commands.push_back(NewCommand);
    }

    int GetIterations() const { return Iterations; }
    const std::vector<std::unique_ptr<ASTNode>>& GetCommands() const { return Commands; }

    void Execute() override {
        for (int i = 0; i < Iterations; i++) {
            for (const auto& Command : Commands) {
                Command->Execute();
            }
        }
    }
};

// Origin node
class OriginNode : public ASTNode {
    bool ClearScreen;
    std::vector<std::unique_ptr<ASTNode>>* ASTTree;

public:
    OriginNode(bool ClearScreen, std::vector<std::unique_ptr<ASTNode>>* ASTTree) : ClearScreen(ClearScreen), ASTTree(ASTTree) {}

    bool GetClearScreen() const { return ClearScreen; }

    void Execute() override {
        if (ClearScreen) {
            if (ASTTree) {
                ASTTree->clear();
            }
        }
        Origin();
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