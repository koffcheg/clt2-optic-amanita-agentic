//
// Created by user on 02.10.24.
//

#ifndef CLT_OPTIC_CALCULATIONTRAJECTORY_H
#define CLT_OPTIC_CALCULATIONTRAJECTORY_H
#include "datapro2.h"

double diff_time_in_seconds(const DateTime& start, const DateTime& stop);
double distance(const Measurement &m1, const Measurement &m2);
bool is_in_strobe(const Measurement &center, const Measurement &point,
                  const double& min_radius, const double& max_radius);
void calculation_parameters_linear_least_squares(const Trajectory &trajectory, const DateTime& time0,
                                                 double& sum_x, double& sum_y, double& sum_z,
                                                 double& sum_x_dt, double& sum_y_dt, double& sum_z_dt,
                                                 double& sum_dt, double& sum_dt2, int& N,
                                                 double& denominator);
void calculation_parameters_quadratic_least_squares(const Trajectory &trajectory, const DateTime& time0,
                                                    double& sum_x, double& sum_y, double& sum_z,
                                                    double& sum_x_dt, double& sum_y_dt, double& sum_z_dt,
                                                    double& sum_x_dt2, double& sum_y_dt2, double& sum_z_dt2,
                                                    double& sum_dt, double& sum_dt2, double& sum_dt3, double& sum_dt4,
                                                    int& N, double& denominator);
Measurement extrapolate_position(const Trajectory &trajectory, const DateTime& t_curr_frame);
Measurement extrapolate_position_qm(const Trajectory &trajectory, const DateTime& t_curr_frame);
void calc_dispersion_measurement_3D(const double& std_x, const double& std_y,
                                    const double& Azt_rad, const double& Elt_rad,
                                    const double& focal_length_x, const double& focal_length_y,
                                    double&  dispersion_xp, double&  dispersion_yp,double&  dispersion_zp);
TStrobe calc_strobe_linear(const Trajectory &trajectory,
                           const Measurement& center, const double& K,
                           const double&  dispersion_xp, const double&  dispersion_yp, const double&  dispersion_zp);
TStrobe calc_strobe_quadratic(const Trajectory &trajectory,
                              const Measurement& center, const double& K,
                              const double&  dispersion_xp, const double&  dispersion_yp, const double&  dispersion_zp);
void calc_dispersion_measurement_3D_binocular(const double& std_x, const double& std_y,
                                              const double& fc_1, const double& fc_2, // mean focal length camera 1 & 2
                                              const double& c1x, const double& c1y, // center frame camera 1
                                              const double& c2x, const double& c2y, // center frame camera 2
                                              const double& xMes_1, const double& yMes_1, // coordinates of the object in the frame of the first camera
                                              const double& xMes_2, const double& yMes_2, // coordinates of the object in the frame of the second camera
                                              const double& zm,                     // scale, distance plane of two frame
                                              const cv::Mat& Rmat_w_to_c1, const cv::Mat& tvec_w_to_c1,
                                              const cv::Mat& Rmat_w_to_c2, const cv::Mat& tvec_w_to_c2,
                                              double&  dispersion_xp, double&  dispersion_yp, double&  dispersion_zp);

#endif //CLT_OPTIC_CALCULATIONTRAJECTORY_H
