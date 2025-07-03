#include "Turtle.hpp"
#include <cmath>

TurtleState::TurtleState(float PastX, float PastY, float StartingX, float StartingY, float StartingDirection, bool StartingPenDown) :
PreviousX(PastX), PreviousY(PastY), CurrentX(StartingX), CurrentY(StartingY), Direction(StartingDirection), PenDown(StartingPenDown) {}

void TurtleState::Origin() {
    PreviousX = WindowWidth / 2.0f;
    PreviousY = WindowHeight / 2.0f;
    CurrentX = WindowWidth / 2.0f;
    CurrentY = WindowHeight / 2.0f;
    Direction = 0;
}

void TurtleState::ClearScreen() {
    ImageLines->clear();
}

void TurtleState::PenState(bool CurrentPen) {
    PenDown = CurrentPen;
}

void TurtleState::MovePosition(float Distance) {
    PreviousX = CurrentX;
    PreviousY = CurrentY;
    CurrentX += Distance * sin((Direction * 3.14)/180);
    CurrentY -= Distance * cos((Direction * 3.14)/180);

    if (ImageLines && PenDown) {
        ImageLines->push_back({sf::Vector2f(PreviousX, PreviousY), sf::Vector2f(CurrentX, CurrentY)});
    }
}

void TurtleState::Turn(float Angle) {
    Direction += Angle;

    if (Direction < 0) { 
        Direction += 360;
    } 
    Direction = std::fmod(Direction, 360.0f);
}