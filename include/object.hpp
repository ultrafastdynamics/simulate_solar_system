#pragma once

#include <string>
#include "position.hpp"

class Object
{
    public :

    Object(double x, double y, double v_x, double v_y, double mass, std::string name);

    Position getPosition() const;

    Position getVelocity() const;

    double getX() const;

    double getY() const;

    double getVx() const;

    double getVy() const;

    double getMass() const;

    const std::string& getName() const;

    void setPosition(const Position& pos);
    void setVelocity(const Position& vel);

    void addPosition(const Position& pos);
    void addVelocity(const Position& vel);

    void setX(double x);
    void setY(double y);
    void setVx(double v_x);
    void setVy(double v_y);

    protected :

    Position position_{};
    Position velocity_{};
    double mass_{1.};
    std::string name_{""};
};
