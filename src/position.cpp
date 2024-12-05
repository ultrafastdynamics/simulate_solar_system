#include "position.hpp"
#include <cmath>

Position::Position(double x, double y) : x_(x), y_(y)
{
}

double Position::distanceTo(const Position& other) const
{
    double x_diff = x_ - other.x_;
    double y_diff = y_ - other.y_;
    return std::sqrt(x_diff * x_diff + y_diff * y_diff);
}

bool Position::operator==(const Position& rhs)
{
    return x_ == rhs.x_ && y_ == rhs.y_;
}

Position Position::operator+(const Position& rhs)
{
    return Position(x_ + rhs.x_, y_ + rhs.y_);
}

Position& Position::operator+=(const Position& rhs)
{
    x_ += rhs.x_;
    y_ += rhs.y_;
    return *this;
}

Position Position::operator-(const Position& rhs)
{
    return Position(x_ - rhs.x_, y_ - rhs.y_);
}

Position& Position::operator-=(const Position& rhs)
{
    x_ -= rhs.x_;
    y_ -= rhs.y_;
    return *this;
}

Position Position::operator*(double factor)
{
    return Position(x_ * factor, y_ * factor);
}

Position& Position::operator*=(double factor)
{
    x_ *= factor;
    y_ *= factor;
    return *this;
}

std::ostream& operator<<(std::ostream& stream, const Position& pos)
{
    stream << pos.x_ << "\t" << pos.y_;
    return stream;
}
