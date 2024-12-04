#pragma once

#include <vector>
#include "object.hpp"
#include "position.hpp"

Position getForce(const Object& o1, const Object& o2);

class Simulation
{
    public :

    Simulation(const std::vector<Object>& objects);

    void runTillNextSave();

    void setTimestep(double timestep);

    void setTimestepInDays(double timestep);

    void setDaysBetweenSaves(double days);

    const std::vector<Object>& getObjects() const;

    std::vector<double> getState() const;

    protected :

    virtual void doTimestep() = 0;

    void calcForces(std::vector<Position>& vec);

    void storeVelocities(std::vector<Position>& vec);

    void storePositions(std::vector<Position>& vec);

    void calculateKepler2();

    std::vector<Object> objects_{};

    double timestep_{1.};

    double current_time_{0.};

    double days_between_saves_{1.};

    int steps_between_saves_{1000};

    double kepler2_area_{0};
    Position kepler2_old_position_planet_{};
};