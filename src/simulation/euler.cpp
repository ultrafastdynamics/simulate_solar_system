#include "simulation/euler.hpp"
#include <QTextStream>

SimulationEuler::SimulationEuler(const std::vector<Object>& objects) : Simulation(objects)
{
    forces_ = std::vector<Position>(objects_.size());
    //QTextStream out(stdout);
    //QString a { "Euler" };
    //out<<a;
}

void SimulationEuler::doTimestep()
{
    calcForces(forces_);
    for(size_t i = 0; i < objects_.size(); i++)
    {
        objects_[i].addPosition(objects_[i].getVelocity() * timestep_);
        objects_[i].addVelocity(forces_[i] * (timestep_ / objects_[i].getMass()));
    }
}
