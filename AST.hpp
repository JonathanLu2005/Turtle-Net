#pragma once
#include <string>
#include <memory>
#include "Turtle.hpp"
#include <vector>

class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void Execute(TurtleState& Turtle) = 0;
    virtual std::string ToString() const = 0;
};

class MovementNode : public ASTNode {
    float Steps;

public:
    MovementNode(float Steps);
    float GetSteps() const;
    void Execute(TurtleState& Turtle) override;
    std::string ToString() const override;
};

class DirectionNode : public ASTNode {
    float Angle;

public:
    DirectionNode(float Angle);
    float GetAngle() const;
    void Execute(TurtleState& Turtle) override;
    std::string ToString() const override;
};

class PenNode : public ASTNode {
    bool PenInUse;

public:
    PenNode(bool PenInUse);
    bool GetPenStatus() const;
    void Execute(TurtleState& Turtle) override;
    std::string ToString() const override;
};

class LoopNode : public ASTNode {
    int Iterations;
    std::vector<std::shared_ptr<ASTNode>> Commands;

public:
    LoopNode(int Iterations);
    void AddCommand(std::shared_ptr<ASTNode> NewCommand);

    int GetIterations() const;
    const std::vector<std::shared_ptr<ASTNode>>& GetCommands() const;
    void Execute(TurtleState& Turtle);
    std::string ToString() const override;
};

class OriginNode : public ASTNode {
    bool ClearScreen;

public:
    OriginNode(bool ClearScreen);
    bool GetClearScreen() const;
    void Execute(TurtleState& Turtle) override;
    std::string ToString() const override;
};

class CommentNode : public ASTNode {
    std::string Comment;

public:
    CommentNode(const std::string& Comment);
    std::string GetComment() const;
    void Execute(TurtleState& Turtle) override;
    std::string ToString() const override;
};