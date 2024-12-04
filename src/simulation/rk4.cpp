#include "simulation/rk4.hpp"
#include <QTextStream>

SimulationRK4::SimulationRK4(const std::vector<Object>& objects)
: Simulation(objects),
position_backup_(objects.size()), velocities_k1_(objects.size()),
velocities_k2_(objects.size()), velocities_k3_(objects.size()),
velocities_k4_(objects.size()), forces_k1_(objects.size()),
forces_k2_(objects.size()), forces_k3_(objects.size()),
forces_k4_(objects.size())
{
    //QTextStream out(stdout);
    //QString a { "RK4" };
    //out<<a;
}

void SimulationRK4::doTimestep()
{
    calcForces(forces_k1_);
    storeVelocities(velocities_k1_);
    storePositions(position_backup_);
    for(size_t i = 0; i < objects_.size(); i++)
    {
        objects_[i].addPosition(velocities_k1_[i] * (timestep_ / 2.));
        objects_[i].addVelocity(forces_k1_[i] * (timestep_ / objects_[i].getMass() / 2.));
    }
    calcForces(forces_k2_);
    storeVelocities(velocities_k2_);
    for(size_t i = 0; i < objects_.size(); i++)
    {
        objects_[i].setPosition(position_backup_[i] + velocities_k2_[i] * (timestep_ / 2.));
        objects_[i].setVelocity(velocities_k1_[i] + forces_k2_[i] * (timestep_ / objects_[i].getMass() / 2.));
    }
    calcForces(forces_k3_);
    storeVelocities(velocities_k3_);
    for(size_t i = 0; i < objects_.size(); i++)
    {
        objects_[i].setPosition(position_backup_[i] + velocities_k3_[i] * timestep_);
        objects_[i].setVelocity(velocities_k1_[i] + forces_k3_[i] * (timestep_ / objects_[i].getMass()));
    }
    calcForces(forces_k4_);
    storeVelocities(velocities_k4_);

    for(size_t i = 0; i < objects_.size(); i++)
    {
        objects_[i].setPosition(position_backup_[i] + (velocities_k1_[i] + velocities_k2_[i] * 2. + velocities_k3_[i] * 2 + velocities_k4_[i]) * (timestep_ / 6.));
        objects_[i].setVelocity(velocities_k1_[i] + (forces_k1_[i] + forces_k2_[i] * 2. + forces_k3_[i] * 2 + forces_k4_[i]) * (timestep_ / 6. / objects_[i].getMass()));
    }
}
