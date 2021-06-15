#include "time/Waveform.hpp"
#include <algorithm>
#include "logging/LogMacros.hpp"
#include "utils/EigenHelperFunctions.hpp"

namespace precice {
namespace time {

Waveform::Waveform(
    int numberOfData,
    int extrapolationOrder,
    int interpolationOrder)
{
  /**
     * Reserve storage depending on required extrapolation order. Extrapolation happens in-place. Therefore, for zeroth
     * order extrapolation we need one column (to read from and write to), for first order two, for second order three. 
     * Note that extrapolationOrder = 0 is an exception, since we want to always work with at least two samples. One at
     * the beginning and one at the end of the time window. Therefore, we use 2 samples for zeroth and first order
     * extrapolation.
     */
  int numberOfSamples = std::max({2, extrapolationOrder + 1, interpolationOrder + 1});
  _timeWindows        = Eigen::MatrixXd::Zero(numberOfData, numberOfSamples);
}

void Waveform::store(Eigen::VectorXd data)
{
  this->_timeWindows.col(0) = data;
}

Eigen::VectorXd Waveform::sample(double dt, int timeWindows, int order)
{
  PRECICE_ASSERT(dt >= 0, "Sampling outside of valid range!");
  PRECICE_ASSERT(dt <= 1, "Sampling outside of valid range!");
  // @ todo: Add more logic here? Set order in constructor; keep track of time windows inside class. See https://github.com/precice/precice/pull/1004/files?file-filters%5B%5D=.cmake&file-filters%5B%5D=.hpp#r642223767.
  return this->interpolateData(order, timeWindows, dt);
}

void Waveform::moveToNextWindow(int timeWindows, int order)
{
  // @ todo: Add more logic here? Set order in constructor; keep track of time windows inside class. See https://github.com/precice/precice/pull/1004/files?file-filters%5B%5D=.cmake&file-filters%5B%5D=.hpp#r642223767.
  auto initialGuess = extrapolateData(order, timeWindows);
  utils::shiftSetFirst(this->_timeWindows, initialGuess);
}

const Eigen::MatrixXd &Waveform::lastTimeWindows()
{
  return _timeWindows;
}

Eigen::VectorXd Waveform::extrapolateData(int order, int timeWindows)
{
  Eigen::VectorXd extrapolatedValue;
  if ((order == 0) || (timeWindows < 2 && order > 0)) {
    PRECICE_ASSERT(this->_timeWindows.cols() > 0);
    extrapolatedValue = this->_timeWindows.col(0);
  } else if ((order == 1) || (timeWindows < 3 && order > 1)) { //timesteps is increased before extrapolate is called
    PRECICE_DEBUG("Performing first order extrapolation");
    PRECICE_ASSERT(this->_timeWindows.cols() > 1);
    extrapolatedValue = this->_timeWindows.col(0) * 2.0; // = 2*x^t
    extrapolatedValue -= this->_timeWindows.col(1);      // = 2*x^t - x^(t-1)
  } else if (order == 2) {
    PRECICE_DEBUG("Performing second order extrapolation");
    PRECICE_ASSERT(this->_timeWindows.cols() > 2);
    extrapolatedValue = this->_timeWindows.col(0) * 2.5;  // = 2.5*x^t
    extrapolatedValue -= this->_timeWindows.col(1) * 2.0; // = 2.5*x^t - 2*x^(t-1)
    extrapolatedValue += this->_timeWindows.col(2) * 0.5; // = 2.5*x^t - 2*x^(t-1) + 0.5*x^(t-2)
  } else {
    PRECICE_ASSERT(false, "Extrapolation order is invalid.");
  }
  return extrapolatedValue;
}

Eigen::VectorXd Waveform::interpolateData(int order, int timeWindows, double dt)
{
  Eigen::VectorXd interpolatedValue;
  if (order == 0) {
    // constant interpolation = just use sample at the end of the window: x(dt) = x^t
    PRECICE_ASSERT(this->_timeWindows.cols() > 0);
    interpolatedValue = this->_timeWindows.col(0);
  } else if ((order == 1) || (timeWindows < 2 && order > 1)) {
    // linear interpolation inside window: x(dt) = dt * x^t + (1-dt) * x^(t-1)
    PRECICE_ASSERT(this->_timeWindows.cols() > 1);
    interpolatedValue = this->_timeWindows.col(0) * dt;        // = dt * x^t
    interpolatedValue += this->_timeWindows.col(1) * (1 - dt); // = dt * x^t + (1-dt) * x^(t-1)
  } else if (order == 2) {
    // quadratic interpolation inside window: x(dt) = x^t * (dt^2 + dt)/2 + x^(t-1) * (1-dt^2)+ x^(t-2) * (dt^2-dt)/2
    PRECICE_ASSERT(this->_timeWindows.cols() > 2);
    interpolatedValue = this->_timeWindows.col(0) * (dt + 1) * dt * 0.5;
    interpolatedValue += this->_timeWindows.col(1) * (1 - dt * dt);
    interpolatedValue += this->_timeWindows.col(2) * (dt - 1) * dt * 0.5;
  } else {
    PRECICE_ASSERT(false, "Interpolation order is invalid.");
  }
  return interpolatedValue;
}

} // namespace time
} // namespace precice