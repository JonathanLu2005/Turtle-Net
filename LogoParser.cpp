#include "LogoParser.hpp"

LogoParser::LogoParser () {
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
const std::vector<std::shared_ptr<ASTNode>>& LogoParser::GetAST() const {
    return ASTTree;
}
