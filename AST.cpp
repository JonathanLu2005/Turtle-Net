#include "AST.hpp"
#include <string>
#include <utility>

// Movement node
MovementNode::MovementNode(float Steps) : Steps(Steps) {}
float MovementNode::GetSteps() const { return Steps; }
void MovementNode::Execute(TurtleState& Turtle) {
    Turtle.MovePosition(Steps);
}
std::string MovementNode::ToString() const {
    return "MovementNode: " + std::to_string(Steps);
}

// Direction node
DirectionNode::DirectionNode(float Angle) : Angle(Angle) {}

float DirectionNode::GetAngle() const { return Angle; }

void DirectionNode::Execute(TurtleState& Turtle) {
    Turtle.Turn(Angle);
}

std::string DirectionNode::ToString() const {
    return "DirectionNode: " + std::to_string(Angle);
}


// Pen node
PenNode::PenNode(bool PenInUse) : PenInUse(PenInUse) {}

bool PenNode::GetPenStatus() const { return PenInUse; }

void PenNode::Execute(TurtleState& Turtle) {
    Turtle.PenState(PenInUse);
}

std::string PenNode::ToString() const {
    return "PenNode: " + std::string(PenInUse ? "Down" : "Up");
}


// Loop node
LoopNode::LoopNode(int Iterations) : Iterations(Iterations) {}

void LoopNode::AddCommand(std::shared_ptr<ASTNode> NewCommand) {
    Commands.push_back(std::move(NewCommand));
}

int LoopNode::GetIterations() const { return Iterations; }
const std::vector<std::shared_ptr<ASTNode>>& LoopNode::GetCommands() const { return Commands; }

void LoopNode::Execute(TurtleState& Turtle) {
    for (int i = 0; i < Iterations; i++) {
        for (auto& Command : Commands) {
            Command->Execute(Turtle);
        }
    }
}

std::string LoopNode::ToString() const {
    std::string Format = "IterationNodes: " + std::to_string(Iterations) + " [";

    for (const auto& Command : Commands) {
        Format += "\n  " + Command->ToString();
    }
    Format += "\n]";

    return Format;
}


// Origin node
OriginNode::OriginNode(bool ClearScreen) : ClearScreen(ClearScreen) {}

bool OriginNode::GetClearScreen() const { return ClearScreen; }

void OriginNode::Execute(TurtleState& Turtle) {
    Turtle.Origin();

    if (ClearScreen) {
        Turtle.ClearScreen();
    }
}

std::string OriginNode::ToString() const {
    return "OriginNode: " + std::string(ClearScreen ? "Clear Screen" : "Home");
}


// Comment node
CommentNode::CommentNode(const std::string& Comment) : Comment(Comment) {}

std::string CommentNode::GetComment() const { return Comment; }

void CommentNode::Execute(TurtleState& Turtle) {}

std::string CommentNode::ToString() const {
    return "CommentNode: " + Comment;
}
