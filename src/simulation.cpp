#include "simulation.hpp"
#include <cmath>
#include <algorithm>
#include <cassert>

#define _USE_MATH_DEFINES

static const double g_constant = 6.674e-11;
static const double seconds_per_day = 86400;

Position getForce(const Object& o1, const Object& o2)
{
    double x_diff = o2.getX() - o1.getX();
    double y_diff = o2.getY() - o1.getY();
    double r_squared = x_diff * x_diff + y_diff * y_diff;
    if(r_squared == 0.)
    {
        return Position(0, 0);
    }
    double r = std::sqrt(r_squared);
    double force = g_constant * o1.getMass() * o2.getMass() / r_squared;
    return Position(force * x_diff / r, force * y_diff / r);
}

Simulation::Simulation(const std::vector<Object>& objects)
    : objects_(objects)
{
}

void Simulation::runTillNextSave()
{
    kepler2_area_ = 0;
    for(int i = 0; i < steps_between_saves_; i++)
    {
        doTimestep();
        calculateKepler2();
        current_time_ += timestep_;
    }
}

void Simulation::setTimestep(double timestep)
{
    timestep_ = timestep;
    steps_between_saves_ = std::max(static_cast<int>(days_between_saves_ * seconds_per_day / timestep_), 1);
}

void Simulation::setTimestepInDays(double timestep)
{
    setTimestep(timestep * seconds_per_day);
}

void Simulation::setDaysBetweenSaves(double days)
{
    days_between_saves_ = days;
    steps_between_saves_ = std::max(static_cast<int>(days_between_saves_ * seconds_per_day / timestep_), 1);
}

const std::vector<Object>& Simulation::getObjects() const
{
    return objects_;
}

std::vector<double> Simulation::getState() const
{
    std::vector<double> state;
    state.push_back(current_time_);
    state.push_back(kepler2_area_);
    for(auto& o : objects_)
    {
        state.push_back(o.getX());
        state.push_back(o.getY());
        state.push_back(o.getVx());
        state.push_back(o.getVy());
    }
    return state;
}

void Simulation::calcForces(std::vector<Position>& vec)
{
    assert(vec.size() == objects_.size());
    std::fill(vec.begin(), vec.end(), Position(0., 0.));
    for(size_t i = 0; i < objects_.size(); i++)
    {
        for(size_t j = i + 1; j < objects_.size(); j++)
        {
            auto force = getForce(objects_[i], objects_[j]);
            vec[i] += force;
            vec[j] -= force;
        }
    }
}

void Simulation::storeVelocities(std::vector<Position>& vec)
{
    assert(vec.size() == objects_.size());
    for(size_t i = 0; i < objects_.size(); i++)
    {
        vec[i] = objects_[i].getVelocity();
    }
}

void Simulation::storePositions(std::vector<Position>& vec)
{
    assert(vec.size() == objects_.size());
    for(size_t i = 0; i < objects_.size(); i++)
    {
        vec[i] = objects_[i].getPosition();
    }
}

void Simulation::calculateKepler2(){
    if(objects_.size()<2){
        return;
    }
    if(current_time_ > 0.){
        double distance = objects_[0].getPosition().distanceTo(objects_[1].getPosition());
        double distance_planet = objects_[1].getPosition().distanceTo(kepler2_old_position_planet_);
        kepler2_area_ += 0.5*distance*distance_planet;
    }
    kepler2_old_position_planet_= objects_[1].getPosition();
}