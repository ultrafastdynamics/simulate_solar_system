#include "object.hpp"

Object::Object(double x, double y, double v_x, double v_y, double mass, std::string name)
: position_(x, y), velocity_(v_x, v_y), mass_(mass), name_(name)
{
}

Position Object::getPosition() const
{
    return position_;
}

Position Object::getVelocity() const
{
    return velocity_;
}

double Object::getX() const
{
    return position_.x_;
}

double Object::getY() const
{
    return position_.y_;
}

double Object::getVx() const
{
    return velocity_.x_;
}

double Object::getVy() const
{
    return velocity_.y_;
}

double Object::getMass() const
{
    return mass_;
}

const std::string& Object::getName() const
{
    return name_;
}

void Object::setPosition(const Position& pos)
{
    position_ = pos;
}

void Object::setVelocity(const Position& vel)
{
    velocity_ = vel;
}

void Object::addPosition(const Position& pos)
{
    position_ += pos;
}

void Object::addVelocity(const Position& vel)
{
    velocity_ += vel;
}

void Object::setX(double x)
{
    position_.x_ = x;
}

void Object::setY(double y)
{
    position_.y_ = y;
}

void Object::setVx(double v_x)
{
    velocity_.x_ = v_x;
}

void Object::setVy(double v_y)
{
    velocity_.y_ = v_y;
}
