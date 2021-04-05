#include <math.h>


namespace Kalman {
    const int SENSORS  = 6;
    const float CHANGING_SPEED = 0.01;
    const float ERROR_MEASURE = 0.1;
    const float ERROR_ESTIMATE = 1;
    
    float err_measure[SENSORS] = {ERROR_MEASURE}; // values spread
    float err_estimate[SENSORS] = {ERROR_ESTIMATE};  // estimate spread
    float last_estimate[SENSORS] = {0};
    float kalman_gain[SENSORS] = {0};

    inline float filter(float value, int key) {
        kalman_gain[key] = err_estimate[key] / (err_estimate[key] + err_measure[key]);
        float current_estimate = last_estimate[key] + kalman_gain[key] * (value - last_estimate[key]);
        err_estimate[key] =  (1.0 - kalman_gain[key]) * err_estimate[key] + fabs(last_estimate[key] - current_estimate) * CHANGING_SPEED;
        last_estimate[key] = current_estimate;
        //return value;
        return current_estimate;
    }
}