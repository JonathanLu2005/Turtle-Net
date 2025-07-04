#pragma once
#include <string>
#include <vector>
#include "Turtle.hpp"
#include "AST.hpp"
#include "LogoParser.hpp"

class LogoEngine {
public:
    std::vector<DrawnLines> Run(const std::string& LogoCode);
};