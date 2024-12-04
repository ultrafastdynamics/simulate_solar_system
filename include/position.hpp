#pragma once
#include <ostream>

struct Position
{
    Position(double x = 0, double y = 0);

    double distanceTo(const Position& other) const;

    Position operator +(const Position& other);
    Position operator -(const Position& other);
    Position operator *(double factor);
    Position& operator +=(const Position& other);
    Position& operator -=(const Position& other);
    Position& operator *=(double factor);

    friend std::ostream& operator<<(std::ostream& stream, const Position& pos);

    double x_;
    double y_;
};
