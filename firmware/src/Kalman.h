#include <math.h>
#define ERR_MEAS 0.01
namespace Kalman {
    float err_measure[9] = {ERR_MEAS, ERR_MEAS, ERR_MEAS, ERR_MEAS, ERR_MEAS, ERR_MEAS, ERR_MEAS, ERR_MEAS, ERR_MEAS}; // values spread
    float err_estimate[9] = {1, 1, 1, 1, 1, 1, 1, 1, 1};  // estimate spread
    float qf = 0.01;  // values changing speed
    float last_estimate[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    float kalman_gain[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

    inline float filter(float value, int key) {
        kalman_gain[key] = err_estimate[key] / (err_estimate[key] + err_measure[key]);
        float current_estimate = last_estimate[key] + kalman_gain[key] * (value - last_estimate[key]);
        err_estimate[key] =  (1.0 - kalman_gain[key]) * err_estimate[key] + fabs(last_estimate[key] - current_estimate) * qf;
        last_estimate[key] = current_estimate;
        return value;
        //return current_estimate;
    }
}