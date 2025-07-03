#pragma once
#define SFML_STATIC
#include <SFML/Graphics.hpp>
#include <vector>

constexpr float WindowWidth = 800;
constexpr float WindowHeight = 600;

struct DrawnLines {
    sf::Vector2f LineStart;
    sf::Vector2f LineEnd;
};

class TurtleState {
public:
    std::vector<DrawnLines>* ImageLines;
    float PreviousX, PreviousY, CurrentX, CurrentY, Direction;
    bool PenDown;

    TurtleState(float PastX, float PastY, float StartingX, float StartingY, float StartingDirection, bool StartingPenDown);

    void Origin();
    void ClearScreen();
    void PenState(bool CurrentPen);
    void MovePosition(float Distance);
    void Turn(float Angle);
};