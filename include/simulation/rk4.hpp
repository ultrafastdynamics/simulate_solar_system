#pragma once

#include "simulation.hpp"

class SimulationRK4 : public Simulation
{
    public :
    
    SimulationRK4(const std::vector<Object>& objects);

    protected :

    void doTimestep() override;

    std::vector<Position> position_backup_{};

    std::vector<Position> velocities_k1_{};

    std::vector<Position> velocities_k2_{};

    std::vector<Position> velocities_k3_{};

    std::vector<Position> velocities_k4_{};

    std::vector<Position> forces_k1_{};

    std::vector<Position> forces_k2_{};

    std::vector<Position> forces_k3_{};

    std::vector<Position> forces_k4_{};
};
