#pragma once
#include <vector>
#include <memory>
#include <string>
#include "AST.hpp"
#include <boost/spirit/include/qi.hpp>
#include <boost/phoenix.hpp>

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

class LogoParser {
public:
    std::vector<std::shared_ptr<ASTNode>> ASTTree;
    using Iterator = std::string::const_iterator;    

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

    LogoParser();

    const std::vector<std::shared_ptr<ASTNode>>& GetAST() const;
};