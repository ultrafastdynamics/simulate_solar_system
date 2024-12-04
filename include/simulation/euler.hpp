#pragma once

#include "simulation.hpp"

class SimulationEuler : public Simulation
{
    public :
    
    SimulationEuler(const std::vector<Object>& objects);

    protected :

    void doTimestep() override;

    std::vector<Position> forces_{};
};
