#pragma once

#include "parameters.hpp"
#include "../../../threading.hpp"

/**
 * @brief Implements the matrix equation
 * 
 */
class DynamicModel
{
public:
    DynamicModel(SimulationData *_pSd) { pSd = _pSd; };
    virtual void ComputeNextStep(int step_ms) = 0;
    virtual void LoadModelParameters(void) = 0;
    virtual Vector3d getPosition(void) = 0;
    virtual Vector3d getAttitude(void) = 0;

protected:
    SimulationData *pSd;
};