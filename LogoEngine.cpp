#include "LogoEngine.hpp"
#include <boost/spirit/include/qi.hpp>
#include <boost/phoenix.hpp>

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

std::vector<DrawnLines> LogoEngine::Run(const std::string& LogoCode) {
    std::vector<DrawnLines> Lines;
    TurtleState MyTurtle(WindowWidth / 2.0f, WindowHeight / 2.0f, WindowWidth / 2.0f, WindowHeight / 2.0f, 0, true);
    MyTurtle.ImageLines = &Lines;

    LogoParser Parser;
    LogoParser::Iterator First = LogoCode.begin();
    LogoParser::Iterator Last = LogoCode.end();
    std::vector<std::shared_ptr<ASTNode>> AST;

    bool ParseResult = qi::phrase_parse(First,Last,Parser.Program >> qi::eoi, ascii::space, AST);

    if (ParseResult && First == Last) {
        Parser.ASTTree = std::move(AST);

        for (const auto& Node : Parser.GetAST()) {
            if (Node) {
                Node->Execute(MyTurtle);
            }
        }
    } else {
        std::cerr << "Fail" << std::endl;
    }

    return Lines;
}