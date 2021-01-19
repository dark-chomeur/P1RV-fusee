#include <cmath>

#include "cardan-model.hpp"

inline cardan_rotation_t Rx(double angle)
{
    cardan_rotation_t dStatedt;
    dStatedt << 1.0, 0.0, 0.0, 0.0, cos(angle), -sin(angle), 0.0, sin(angle),
        cos(angle);
    return dStatedt;
}

inline cardan_rotation_t Ry(double angle)
{
    cardan_rotation_t dStatedt;
    dStatedt << cos(angle), 0.0, sin(angle), 0.0, 1.0, 0.0, -sin(angle), 0.0,
        cos(angle);
    return dStatedt;
}

inline cardan_rotation_t Rz(double angle)
{
    cardan_rotation_t dStatedt;
    dStatedt << cos(angle), -sin(angle), 0.0, sin(angle), cos(angle), 0.0, 0.0, 0.0,
        1.0;
    return dStatedt;
}

inline cardan_rotation_t S(Eigen::Vector3d const &v)
{
    cardan_rotation_t vp;
    vp << 0.0, -v(2, 0), v(1, 0), v(2, 0), 0.0, -v(0, 0), -v(1, 0), v(0, 0), 0.0;
    return vp;
}

inline cardan_action_t TauG(cardan_rotation_t const &j1, cardan_state_t const &state)
{
    double m_mass = 0;
    double m_g = 9.8;
    Eigen::Vector3d m_weight(0.0, 0.0, m_mass * m_g);

    double altitude = state(8, 0);
    double mass = 0;
    cardan_action_t tauG;

    tauG.block<3, 1>(0, 0) = j1.transpose() * m_weight;
    tauG.block<3, 1>(3, 0) << 0, 0, 0;

    return tauG;
}

inline cardan_action_t TauATilde(cardan_state_t const &state)
{
    cardan_action_t dStatedt;

    return dStatedt;
}

inline cardan_action_t TauD(cardan_state_t const &state)
{
    cardan_action_t tauD;

    return tauD;
}

inline cardan_action_t TauC(cardan_state_t const &state, cardan_command_t const &command)
{
    cardan_action_t tauC;
    tauC.block<3, 1>(0, 0) << command(0), 0, 0;
    tauC.block<3, 1>(0, 0) = Rz(command(2)) * Ry(command(1)) * tauC.block<3, 1>(0, 0);
    Eigen::Vector3d nozzle;
    nozzle << -0.50, 0, 0;
    tauC.block<3, 1>(3, 0) = S(nozzle) * tauC.block<3, 1>(0, 0);

    return tauC;
}

Eigen::Matrix<double, 6, 6> RBCoriolisMatrix(cardan_state_t const &state)
{
    Eigen::Matrix<double, 6, 6> dStatedt;

    return dStatedt;
}

Eigen::Matrix<double, 6, 6> AddedMassMatrix(const double &depth)
{
    Eigen::Matrix<double, 6, 6> dStatedt;

    return dStatedt;
}

Eigen::Matrix<double, 6, 6> M(double altitude)
{
    Eigen::Matrix<double, 6, 6> m_m_rb;
    return m_m_rb + AddedMassMatrix(altitude);
}

void CardanModel::ComputeStateDerivative(void)
{

    cardan_rotation_t j_1 =
        Rz(state(11, 0)) * Ry(state(10, 0)) * Rx(state(9, 0));
    cardan_rotation_t j_2;

    j_2 << 1.0, sin(state(9, 0)) * tan(state(10, 0)),
        cos(state(9, 0)) * tan(state(10, 0)), 0.0, cos(state(9, 0)),
        -sin(state(9, 0)), 0, sin(state(9, 0)) / cos(state(10, 0)),
        cos(state(9, 0)) / cos(state(10, 0));

    cardan_action_t nu = state.block<6, 1>(0, 0);

    cardan_action_t tau_tilde = TauG(j_1, state) /* + TauATilde(state) */ /* + TauD(state) */ + TauC(state, command);

    dStatedt.block<6, 1>(0, 0) =
        M(state(8, 0)).inverse() * (tau_tilde - RBCoriolisMatrix(state) * nu);
    dStatedt.block<3, 1>(6, 0) = j_1 * nu.block<3, 1>(0, 0);
    dStatedt.block<3, 1>(9, 0) = j_2 * nu.block<3, 1>(3, 0);
}

void CardanModel::ComputeNextState(double step_fraction)
{
    state += step_fraction * step_ms / 1000.0 * dStatedt;
}

void CardanModel::LoadModelParameters(void)
{
    // While there is no proper loader, paramaters are hard coded
}

Vector3d CardanModel::getPosition(void)
{
    Vector3d v;
    v.x = state(6);
    v.y = state(7);
    v.z = state(8);
    return v;
}

void CardanModel::SetPosition(Vector3d position)
{
    state.block<3, 1>(6, 0) << position.x, position.y, position.z;
}

Vector3d CardanModel::getAttitude(void)
{
    Vector3d v;
    v.x = state(9);
    v.y = state(10);
    v.z = state(11);
    return v;
}

void CardanModel::SetAttitude(Vector3d attitude)
{
    state.block<3, 1>(9, 0) << attitude.x, attitude.y, attitude.z;
}

void CardanModel::UpdateCommand(SimulationData *pSd)
{
    command(0) = pSd->pShm->booster_thrust;
    command(1) = pSd->pShm->tvc_angle_1;
    command(2) = pSd->pShm->tvc_angle_2;
}

void CardanModel::BufferizeState(int bufferIndex)
{
    buffer[bufferIndex] = state;
}

void CardanModel::LoadState(int bufferIndex)
{
    state = buffer[bufferIndex];
}

void CardanModel::BufferizeStateDerivative(int bufferIndex)
{
    buffer[bufferIndex] = dStatedt;
}

void CardanModel::LoadStateDerivative(int bufferIndex)
{
    dStatedt = buffer[bufferIndex];
}

void CardanModel::ClearBuffer(void)
{
    buffer.clear();
}