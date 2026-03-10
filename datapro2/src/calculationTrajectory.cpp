//
// Created by user on 02.10.24.
//
#include "calculationTrajectory.h"
#include <log4cxx/logger.h>
static auto logger = log4cxx::Logger::getLogger("calc-traj");

double diff_time_in_seconds(const DateTime& start, const DateTime& stop){
    std::chrono::duration<double> DT1 = stop - start;
    return DT1.count();
}

double distance(const Measurement &m1, const Measurement &m2) {
    return std::sqrt(std::pow(m1.Xp - m2.Xp, 2) + std::pow(m1.Yp - m2.Yp, 2) + std::pow(m1.Zp - m2.Zp, 2));
}

bool is_in_strobe(const Measurement &center, const Measurement &point,
                  const double& min_radius, const double& max_radius) {//, const bool& coordinate_system_2D
    double dist;
    dist = distance(center, point);
    return dist >= min_radius && dist <= max_radius;
}

double diff_coor(const double &x_center, const double &x_point) {
    return (x_point - x_center);
}

void calculation_parameters_linear_least_squares(const Trajectory &trajectory, const DateTime& time0,
                                                 double& sum_x, double& sum_y, double& sum_z,
                                                 double& sum_x_dt, double& sum_y_dt, double& sum_z_dt,
                                                 double& sum_dt, double& sum_dt2, int& N,
                                                 double& denominator){
    double dt;
    for (const auto &m: trajectory.measurements) {
        dt = diff_time_in_seconds(time0, m.time);
        sum_x += m.Xp;
        sum_y += m.Yp;
        sum_z += m.Zp;
        sum_x_dt += m.Xp * dt;
        sum_y_dt += m.Yp * dt;
        sum_z_dt += m.Zp * dt;
        sum_dt += dt;
        sum_dt2 += dt * dt;
    }
    N = (int)trajectory.measurements.size();
    denominator = (N) * sum_dt2 - sum_dt * sum_dt;
}

void calculation_parameters_quadratic_least_squares(const Trajectory &trajectory, const DateTime& time0,
                                                    double& sum_x, double& sum_y, double& sum_z,
                                                    double& sum_x_dt, double& sum_y_dt, double& sum_z_dt,
                                                    double& sum_x_dt2, double& sum_y_dt2, double& sum_z_dt2,
                                                    double& sum_dt, double& sum_dt2, double& sum_dt3, double& sum_dt4,
                                                    int& N, double& denominator){
    double dt2, dt;
    for (const auto &m: trajectory.measurements) {
        dt = diff_time_in_seconds(time0, m.time);
        dt2 = dt * dt;
        sum_x += m.Xp;
        sum_y += m.Yp;
        sum_z += m.Zp;
        sum_x_dt += m.Xp * dt;
        sum_y_dt += m.Yp * dt;
        sum_z_dt += m.Zp * dt;
        sum_x_dt2 += m.Xp * dt2;
        sum_y_dt2 += m.Yp * dt2;
        sum_z_dt2 += m.Zp * dt2;
        sum_dt += dt;
        sum_dt2 += dt2;
        sum_dt3 += dt2 * dt;
        sum_dt4 += dt2 * dt2;
    }
    N = (int)trajectory.measurements.size();
    denominator = sum_dt4 * sum_dt * sum_dt - 2 * sum_dt * sum_dt2 * sum_dt3 +
                  sum_dt2 * sum_dt2 * sum_dt2 - N * sum_dt4 * sum_dt2 + N * sum_dt3 * sum_dt3;
}

void calc_dispersion_parameters_linear_model(const double& sum_dt, const double& sum_dt2, const int& N, const double& denominator,
                                             double& norm_dispersion_0, double& norm_dispersion_V){

    norm_dispersion_0 = 1 / (denominator * denominator) *
            (N * sum_dt2 * sum_dt2 + sum_dt * sum_dt * sum_dt2);
    norm_dispersion_V = 1 / (denominator * denominator) *
            (N * N * sum_dt2 + sum_dt * sum_dt * N);
}

void calc_dispersion_parameters_linear_model(const Trajectory &trajectory, const DateTime& time0,
                                             const double&  dispersion_xp, const double&  dispersion_yp, const double&  dispersion_zp,
                                             double& dispersion_x0, double& dispersion_Vx,
                                             double& dispersion_y0, double& dispersion_Vy,
                                             double& dispersion_z0, double& dispersion_Vz){
    double sum_x=0, sum_y=0, sum_z=0,
            sum_x_dt=0, sum_y_dt=0, sum_z_dt=0,
            sum_dt=0, sum_dt2=0, denominator=0,
            norm_dispersion_0, norm_dispersion_V;
    int N;
    calculation_parameters_linear_least_squares(trajectory, time0,
                                                sum_x, sum_y, sum_z,
                                                sum_x_dt, sum_y_dt,sum_z_dt,
                                                sum_dt, sum_dt2, N, denominator);
    calc_dispersion_parameters_linear_model(sum_dt, sum_dt2, N, denominator,
                                            norm_dispersion_0, norm_dispersion_V);
    dispersion_x0 = dispersion_xp * norm_dispersion_0;
    dispersion_Vx = dispersion_xp * norm_dispersion_V;
    dispersion_y0 = dispersion_yp * norm_dispersion_0;
    dispersion_Vy = dispersion_yp * norm_dispersion_V;
    dispersion_z0 = dispersion_zp * norm_dispersion_0;
    dispersion_Vz = dispersion_zp * norm_dispersion_V;
}

double calc_std_quadratic_model(const double& sum_dt2, const double& sum_dt3,
                                const double& sum_dt4, const double& denominator){
    return sqrt((sum_dt3 * sum_dt3 - sum_dt2 * sum_dt4)/ denominator);
}

void calc_dispersion_parameters_quadratic_model(double& sum_dt, double& sum_dt2, double& sum_dt3, double& sum_dt4,
                                                int& N, double& denominator,
                                                double& norm_dispersion_0, double& norm_dispersion_V, double& norm_dispersion_a){

    norm_dispersion_0 = 1 / (denominator * denominator) *
                        (sum_dt4 * pow(sum_dt2,4) + sum_dt2 * pow(sum_dt2 * sum_dt3,2) +
                        N * pow(sum_dt4 * sum_dt2,2) + N * pow(sum_dt3,4) +
                        sum_dt4 * pow(sum_dt * sum_dt3,2) + sum_dt2 * pow(sum_dt * sum_dt4,2));
    norm_dispersion_V = 1 / (denominator * denominator) *
                        (pow(sum_dt2,5) + sum_dt2 * pow(N * sum_dt4,4) +
                        sum_dt4 * pow(N * sum_dt3,2) + N * pow(sum_dt * sum_dt4,4) +
                        N * pow(sum_dt2 * sum_dt3,2) + sum_dt2 * pow(sum_dt * sum_dt2,2));
    norm_dispersion_a = 1 / (denominator * denominator) *
                        (sum_dt4 * pow(sum_dt,4) + sum_dt2 * pow(sum_dt * sum_dt2,2) +
                        sum_dt3 * sum_dt * N + pow(sum_dt2,4) * N +
                        sum_dt4 * pow(N * sum_dt2,2) + sum_dt2 * pow(N * sum_dt3,2));
}

void calc_dispersion_params_quadratic_model(const Trajectory &trajectory, const DateTime& time0,
                                            const double&  dispersion_xp, const double&  dispersion_yp, const double&  dispersion_zp,
                                            double& dispersion_x0, double& dispersion_Vx, double& dispersion_ax,
                                            double& dispersion_y0, double& dispersion_Vy, double& dispersion_ay,
                                            double& dispersion_z0, double& dispersion_Vz, double& dispersion_az){
    double sum_x=0, sum_y=0, sum_z=0,
            sum_x_dt=0, sum_y_dt=0, sum_z_dt=0,
            sum_x_dt2 = 0, sum_y_dt2 = 0, sum_z_dt2 = 0,
            sum_dt=0, sum_dt2=0, sum_dt3 = 0, sum_dt4 = 0, denominator=0,
            norm_dispersion_0, norm_dispersion_V, norm_dispersion_a;
    int N;
    calculation_parameters_quadratic_least_squares(trajectory, time0,
                                                   sum_x, sum_y, sum_z,
                                                   sum_x_dt, sum_y_dt,sum_z_dt,
                                                   sum_x_dt2, sum_y_dt2, sum_z_dt2,
                                                   sum_dt, sum_dt2, sum_dt3, sum_dt4, N, denominator);

    calc_dispersion_parameters_quadratic_model(sum_dt, sum_dt2, sum_dt3, sum_dt4, N, denominator,
                                               norm_dispersion_0, norm_dispersion_V, norm_dispersion_a);

    dispersion_x0 = dispersion_xp * norm_dispersion_0;
    dispersion_Vx = dispersion_xp * norm_dispersion_V;
    dispersion_ax = dispersion_xp * norm_dispersion_a;
    dispersion_y0 = dispersion_yp * norm_dispersion_0;
    dispersion_Vy = dispersion_yp * norm_dispersion_V;
    dispersion_ay = dispersion_yp * norm_dispersion_a;
    dispersion_z0 = dispersion_zp * norm_dispersion_0;
    dispersion_Vz = dispersion_zp * norm_dispersion_V;
    dispersion_az = dispersion_zp * norm_dispersion_a;
}

Measurement extrapolate_position(const Trajectory &trajectory, const DateTime& t_curr_frame) {
    double time_diff = diff_time_in_seconds(trajectory.time0, t_curr_frame);

    double next_x = trajectory.x0_hat + trajectory.Vx_hat * time_diff;
    double next_y = trajectory.y0_hat + trajectory.Vy_hat * time_diff;
    double next_z = trajectory.z0_hat + trajectory.Vz_hat * time_diff;
    return Measurement{.Xp = next_x, .Yp = next_y, .Zp = next_z, .time = t_curr_frame};
}

Measurement extrapolate_position_qm(const Trajectory &trajectory, const DateTime& t_curr_frame) {
    double time_diff = diff_time_in_seconds(trajectory.time0, t_curr_frame);

    double next_x = trajectory.x0_hat + trajectory.Vx_hat * time_diff + trajectory.ax_hat * pow(time_diff, 2);
    double next_y = trajectory.y0_hat + trajectory.Vy_hat * time_diff + trajectory.ay_hat * pow(time_diff, 2);
    double next_z = trajectory.z0_hat + trajectory.Vz_hat * time_diff + trajectory.az_hat * pow(time_diff, 2);
    return Measurement{.Xp = next_x, .Yp = next_y, .Zp = next_z, .time = t_curr_frame};
}

void calc_dispersion_measurement_3D(const double& std_x, const double& std_y,
                             const double& Azt_rad, const double& Elt_rad,
                             const double& focal_length_x, const double& focal_length_y,
                             double&  dispersion_xp, double&  dispersion_yp, double&  dispersion_zp){
    double k_x = std_x / focal_length_x, k_y = std_y / focal_length_y;

    dispersion_xp = pow(k_y * sin(Elt_rad) * cos(Azt_rad),2) +
             pow(k_x * sin(Azt_rad),2);
    dispersion_yp = pow(k_y * sin(Elt_rad) * sin(Azt_rad),2) +
             pow(k_x * cos(Azt_rad),2);
    dispersion_zp = pow(k_y * cos(Elt_rad) ,2);
}

double calc_dispersion_extrapolation_linear_model(const double& D_0, const double& D_V, const double& time_diff){
    double De = D_0 + D_V * time_diff * time_diff;
    return De;
}

double check_dispersion(const double& dispersion){
    double no_zero_dispersion = 1E-6;
    if (dispersion < 1E-13)
        return no_zero_dispersion;
    else
        return dispersion;
}

TStrobe calc_strobe_linear(const Trajectory &trajectory,
                           const Measurement& center, const double& K,
                           const double&  dispersion_xp, const double&  dispersion_yp, const double&  dispersion_zp){
    TStrobe strobe{};
    double dispersion_x0 = 0, dispersion_Vx = 0, dispersion_y0 = 0, dispersion_Vy = 0, dispersion_z0 = 0, dispersion_Vz = 0;

    double time_diff = diff_time_in_seconds(trajectory.measurements.back().time, center.time);

    calc_dispersion_parameters_linear_model(trajectory, trajectory.time0,
                                            dispersion_xp, dispersion_yp, dispersion_zp,
                                            dispersion_x0, dispersion_Vx,
                                            dispersion_y0, dispersion_Vy,
                                            dispersion_z0, dispersion_Vz);

    double dispersion_xpe = calc_dispersion_extrapolation_linear_model(dispersion_x0, dispersion_Vx, time_diff);
    double dispersion_ype = calc_dispersion_extrapolation_linear_model(dispersion_y0, dispersion_Vy, time_diff);
    double dispersion_zpe = calc_dispersion_extrapolation_linear_model(dispersion_z0, dispersion_Vz, time_diff);

    dispersion_xpe = check_dispersion(dispersion_xpe);
    dispersion_ype = check_dispersion(dispersion_ype);
    dispersion_zpe = check_dispersion(dispersion_zpe);
    LOG4CXX_DEBUG(logger, "stdXel = " << sqrt(dispersion_xpe) <<
                                      ", stdYel = " << sqrt(dispersion_ype) <<
                                      ", stdZel = " << sqrt(dispersion_zpe));
    double r_x = sqrt(dispersion_xpe) * K,
            r_y = sqrt(dispersion_ype) * K,
            r_z = sqrt(dispersion_zpe) * K;
    {
        strobe.id_trajectory = trajectory.id;
        strobe.x_0 = center.Xp;
        strobe.y_0 = center.Yp;
        strobe.z_0 = center.Zp;
        strobe.x_min = center.Xp - r_x * .5;
        strobe.y_min = center.Yp - r_y * .5;
        strobe.z_min = center.Zp - r_z * .5;
        strobe.x_max = center.Xp + r_x * .5;
        strobe.y_max = center.Yp + r_y * .5;
        strobe.z_max = center.Zp + r_z * .5;
    }
    return strobe;
}

double calc_dispersion_extrapolation_quadratic_model(const double& D_0, const double& D_V, const double& D_a, const double& time_diff){
    double De = D_0 + D_V * pow(time_diff,2) + D_a * pow(time_diff,4);
    return De;
}

TStrobe calc_strobe_quadratic(const Trajectory &trajectory,
                              const Measurement& center, const double& K,
                              const double&  dispersion_xp, const double&  dispersion_yp, const double&  dispersion_zp){
    TStrobe strobe{};
    double dispersion_x0 = 0, dispersion_Vx = 0, dispersion_ax = 0,
    dispersion_y0 = 0, dispersion_Vy = 0, dispersion_ay = 0,
    dispersion_z0 = 0, dispersion_Vz = 0, dispersion_az = 0;

    double time_diff = diff_time_in_seconds(trajectory.measurements.back().time, center.time);

    calc_dispersion_params_quadratic_model(trajectory, trajectory.time0,
                                           dispersion_xp, dispersion_yp, dispersion_zp,
                                           dispersion_x0, dispersion_Vx, dispersion_ax,
                                           dispersion_y0, dispersion_Vy, dispersion_ay,
                                           dispersion_z0, dispersion_Vz, dispersion_az);

    double dispersion_xpe = calc_dispersion_extrapolation_quadratic_model(dispersion_x0, dispersion_Vx, dispersion_ax, time_diff);
    double dispersion_ype = calc_dispersion_extrapolation_quadratic_model(dispersion_y0, dispersion_Vy, dispersion_ay, time_diff);
    double dispersion_zpe = calc_dispersion_extrapolation_quadratic_model(dispersion_z0, dispersion_Vz, dispersion_az, time_diff);

    dispersion_xpe = check_dispersion(dispersion_xpe);
    dispersion_ype = check_dispersion(dispersion_ype);
    dispersion_zpe = check_dispersion(dispersion_zpe);
    LOG4CXX_DEBUG(logger, "stdXeq = " << sqrt(dispersion_xpe) <<
                                     ", stdYeq = " << sqrt(dispersion_ype) <<
                                     ", stdZeq = " << sqrt(dispersion_zpe));

    double r_x = sqrt(dispersion_xpe) * K,
            r_y = sqrt(dispersion_ype) * K,
            r_z = sqrt(dispersion_zpe) * K;
    {
        strobe.id_trajectory = trajectory.id;
        strobe.x_0 = center.Xp;
        strobe.y_0 = center.Yp;
        strobe.z_0 = center.Zp;
        strobe.x_min = center.Xp - r_x * .5;
        strobe.y_min = center.Yp - r_y * .5;
        strobe.z_min = center.Zp - r_z * .5;
        strobe.x_max = center.Xp + r_x * .5;
        strobe.y_max = center.Yp + r_y * .5;
        strobe.z_max = center.Zp + r_z * .5;
    }
    return strobe;
}

void diff_Mxa2(const double& fc_1, const double& fc_2, // mean focal length camera 1 & 2
               const double& c1x, const double& c1y, // center frame camera 1
               const double& c2x, const double& c2y, // center frame camera 2
               const double& xMes_1, const double& yMes_1, // coordinates of the object in the frame of the first camera
               const double& xMes_2, const double& yMes_2, // coordinates of the object in the frame of the second camera
               const double& zm,                     // scale, distance plane of two frame
               const cv::Mat& Rmat_c1_to_w, const cv::Mat& tvec_c1_to_w,
               const cv::Mat& Rmat_c2_to_w, const cv::Mat& tvec_c2_to_w,
               double& d_Mxa2_x, double& d_Mxa2_y, double& d_Mxa2_z) {

    double t_11 = tvec_c1_to_w.at<double>(0), t_12 = tvec_c1_to_w.at<double>(1), t_13 = tvec_c1_to_w.at<double>(2),
            t_21 = tvec_c2_to_w.at<double>(0), t_22 = tvec_c2_to_w.at<double>(1), t_23 = tvec_c2_to_w.at<double>(2);
    double r_111 = Rmat_c1_to_w.at<double>(0, 0),
            r_112 = Rmat_c1_to_w.at<double>(0, 1),
            r_113 = Rmat_c1_to_w.at<double>(0, 2),
            r_121 = Rmat_c1_to_w.at<double>(1, 0),
            r_122 = Rmat_c1_to_w.at<double>(1, 1),
            r_123 = Rmat_c1_to_w.at<double>(1, 2),
            r_131 = Rmat_c1_to_w.at<double>(2, 0),
            r_132 = Rmat_c1_to_w.at<double>(2, 1),
            r_133 = Rmat_c1_to_w.at<double>(2, 2);
    double r_211 = Rmat_c2_to_w.at<double>(0, 0),
            r_212 = Rmat_c2_to_w.at<double>(0, 1),
            r_213 = Rmat_c2_to_w.at<double>(0, 2),
            r_221 = Rmat_c2_to_w.at<double>(1, 0),
            r_222 = Rmat_c2_to_w.at<double>(1, 1),
            r_223 = Rmat_c2_to_w.at<double>(1, 2),
            r_231 = Rmat_c2_to_w.at<double>(2, 0),
            r_232 = Rmat_c2_to_w.at<double>(2, 1),
            r_233 = Rmat_c2_to_w.at<double>(2, 2);

    double sig_18 = (r_111 * zm * (c1x - xMes_1)) / fc_1 - r_113 * zm + (r_112 * zm * (c1y - yMes_1)) / fc_1;
    double sig_17 = (r_121 * zm * (c1x - xMes_1)) / fc_1 - r_123 * zm + (r_122 * zm * (c1y - yMes_1)) / fc_1;
    double sig_16 = (r_211 * zm * (c2x - xMes_2)) / fc_2 - r_213 * zm + (r_212 * zm * (c2y - yMes_2)) / fc_2;
    double sig_15 = (r_221 * zm * (c2x - xMes_2)) / fc_2 - r_223 * zm + (r_222 * zm * (c2y - yMes_2)) / fc_2;
    double sig_14 = (r_131 * zm * (c1x - xMes_1)) / fc_1 - r_133 * zm + (r_132 * zm * (c1y - yMes_1)) / fc_1;
    double sig_13 = (r_231 * zm * (c2x - xMes_2)) / fc_2 - r_233 * zm + (r_232 * zm * (c2y - yMes_2)) / fc_2;
    double sig_12 = sig_18 * sig_15 - sig_16 * sig_17;
    double sig_11 = sig_18 * sig_13 - sig_16 * sig_14;
    double sig_10 = sig_17 * sig_13 - sig_15 * sig_14;
    double sig_9 = (r_121 * zm * sig_16) / fc_1 - (r_111 * zm * sig_15) / fc_1;
    double sig_8 = (r_131 * zm * sig_16) / fc_1 - (r_111 * zm * sig_13) / fc_1;
    double sig_7 = (r_131 * zm * sig_15) / fc_1 - (r_121 * zm * sig_13) / fc_1;
    double sig_6 = sig_12 * sig_12 + sig_11 * sig_11 + sig_10 * sig_10;
    double sig_5 =
            (sig_9 * sig_16 - sig_7 * sig_13) * (t_12 - t_22) - (sig_9 * sig_15 + sig_8 * sig_13) * (t_11 - t_21) +
            (sig_8 * sig_16 + sig_7 * sig_15) * (t_13 - t_23);
    double sig_4 =
            (sig_12 * sig_18 - sig_10 * sig_14) * (t_12 - t_22) - (sig_12 * sig_17 + sig_11 * sig_14) * (t_11 - t_21) +
            (sig_11 * sig_18 + sig_10 * sig_17) * (t_13 - t_23);
    double sig_3 = 2 * sig_12 * sig_9 + 2 * sig_11 * sig_8 + 2 * sig_10 * sig_7;
    double sig_2 = (t_12 - t_22) *
                   (sig_9 * sig_18 - sig_7 * sig_14 - (r_111 * zm * sig_12) / fc_1 + (r_131 * zm * sig_10) / fc_1) -
                   (t_11 - t_21) *
                   (sig_9 * sig_17 + sig_8 * sig_14 - (r_121 * zm * sig_12) / fc_1 - (r_131 * zm * sig_11) / fc_1) +
                   (t_13 - t_23) *
                   (sig_8 * sig_18 + sig_7 * sig_17 - (r_111 * zm * sig_11) / fc_1 - (r_121 * zm * sig_10) / fc_1);
    double sig_1 =
            (sig_12 * sig_16 - sig_10 * sig_13) * (t_12 - t_22) - (sig_12 * sig_15 + sig_11 * sig_13) * (t_11 - t_21) +
            (sig_11 * sig_16 + sig_10 * sig_15) * (t_13 - t_23);
    double sig_sq_6 = sig_6 * sig_6;

    d_Mxa2_x = (sig_16 * sig_2) / (2 * sig_6) + (sig_18 * sig_5) / (2 * sig_6) -
               (sig_16 * sig_3 * sig_4) / (2 * sig_sq_6) -
               (sig_18 * sig_3 * sig_1) / (2 * sig_sq_6) - (r_111 * zm * sig_1) / (2 * fc_1 * sig_6);
    d_Mxa2_y = (sig_15 * sig_2) / (2 * sig_6) + (sig_17 * sig_5) / (2 * sig_6) -
               (sig_15 * sig_3 * sig_4) / (2 * sig_sq_6) - (sig_17 * sig_3 * sig_1) / (2 * sig_sq_6) -
               (r_121 * zm * sig_1) / (2 * fc_1 * sig_6);
    d_Mxa2_z = (sig_13 * sig_2) / (2 * sig_6) + (sig_14 * sig_5) / (2 * sig_6) -
               (sig_13 * sig_3 * sig_4) / (2 * sig_sq_6) - (sig_14 * sig_3 * sig_1) / (2 * sig_sq_6) -
               (r_131 * zm * sig_1) / (2 * fc_1 * sig_6);
}

void diff_Mya2(const double& fc_1, const double& fc_2, // mean focal length camera 1 & 2
               const double& c1x, const double& c1y, // center frame camera 1
               const double& c2x, const double& c2y, // center frame camera 2
               const double& xMes_1, const double& yMes_1, // coordinates of the object in the frame of the first camera
               const double& xMes_2, const double& yMes_2, // coordinates of the object in the frame of the second camera
               const double& zm,                     // scale, distance plane of two frame
               const cv::Mat& Rmat_c1_to_w, const cv::Mat& tvec_c1_to_w,
               const cv::Mat& Rmat_c2_to_w, const cv::Mat& tvec_c2_to_w,
               double& d_Mya2_x, double& d_Mya2_y, double& d_Mya2_z) {

    double t_11 = tvec_c1_to_w.at<double>(0), t_12 = tvec_c1_to_w.at<double>(1), t_13 = tvec_c1_to_w.at<double>(2),
            t_21 = tvec_c2_to_w.at<double>(0), t_22 = tvec_c2_to_w.at<double>(1), t_23 = tvec_c2_to_w.at<double>(2);
    double r_111 = Rmat_c1_to_w.at<double>(0, 0),
            r_112 = Rmat_c1_to_w.at<double>(0, 1),
            r_113 = Rmat_c1_to_w.at<double>(0, 2),
            r_121 = Rmat_c1_to_w.at<double>(1, 0),
            r_122 = Rmat_c1_to_w.at<double>(1, 1),
            r_123 = Rmat_c1_to_w.at<double>(1, 2),
            r_131 = Rmat_c1_to_w.at<double>(2, 0),
            r_132 = Rmat_c1_to_w.at<double>(2, 1),
            r_133 = Rmat_c1_to_w.at<double>(2, 2);
    double r_211 = Rmat_c2_to_w.at<double>(0, 0),
            r_212 = Rmat_c2_to_w.at<double>(0, 1),
            r_213 = Rmat_c2_to_w.at<double>(0, 2),
            r_221 = Rmat_c2_to_w.at<double>(1, 0),
            r_222 = Rmat_c2_to_w.at<double>(1, 1),
            r_223 = Rmat_c2_to_w.at<double>(1, 2),
            r_231 = Rmat_c2_to_w.at<double>(2, 0),
            r_232 = Rmat_c2_to_w.at<double>(2, 1),
            r_233 = Rmat_c2_to_w.at<double>(2, 2);

    double sig_18 = (r_111 * zm * (c1x - xMes_1)) / fc_1 - r_113 * zm + (r_112 * zm * (c1y - yMes_1)) / fc_1;
    double sig_17 = (r_121 * zm * (c1x - xMes_1)) / fc_1 - r_123 * zm + (r_122 * zm * (c1y - yMes_1)) / fc_1;
    double sig_16 = (r_211 * zm * (c2x - xMes_2)) / fc_2 - r_213 * zm + (r_212 * zm * (c2y - yMes_2)) / fc_2;
    double sig_15 = (r_221 * zm * (c2x - xMes_2)) / fc_2 - r_223 * zm + (r_222 * zm * (c2y - yMes_2)) / fc_2;
    double sig_14 = (r_131 * zm * (c1x - xMes_1)) / fc_1 - r_133 * zm + (r_132 * zm * (c1y - yMes_1)) / fc_1;
    double sig_13 = (r_231 * zm * (c2x - xMes_2)) / fc_2 - r_233 * zm + (r_232 * zm * (c2y - yMes_2)) / fc_2;
    double sig_12 = sig_18 * sig_15 - sig_17 * sig_16;
    double sig_11 = sig_18 * sig_13 - sig_14 * sig_16;
    double sig_10 = sig_17 * sig_13 - sig_14 * sig_15;
    double sig_9 = (r_122 * zm * sig_16) / fc_1 - (r_112 * zm * sig_15) / fc_1;
    double sig_8 = (r_132 * zm * sig_16) / fc_1 - (r_112 * zm * sig_13) / fc_1;
    double sig_7 = (r_132 * zm * sig_15) / fc_1 - (r_122 * zm * sig_13) / fc_1;
    double sig_6 = sig_12 * sig_12 + sig_11 * sig_11 + sig_10 * sig_10;
    double sig_5 =
            (sig_9 * sig_16 - sig_7 * sig_13) * (t_12 - t_22) - (sig_9 * sig_15 + sig_8 * sig_13) * (t_11 - t_21) +
            (sig_8 * sig_16 + sig_7 * sig_15) * (t_13 - t_23);
    double sig_4 =
            (sig_12 * sig_18 - sig_10 * sig_14) * (t_12 - t_22) - (sig_12 * sig_17 + sig_11 * sig_14) * (t_11 - t_21) +
            (sig_11 * sig_18 + sig_10 * sig_17) * (t_13 - t_23);
    double sig_3 = 2 * sig_12 * sig_9 + 2 * sig_11 * sig_8 + 2 * sig_10 * sig_7;
    double sig_2 = (t_12 - t_22) *
                   (sig_9 * sig_18 - sig_7 * sig_14 - (r_112 * zm * sig_12) / fc_1 + (r_132 * zm * sig_10) / fc_1) -
                   (t_11 - t_21) *
                   (sig_9 * sig_17 + sig_8 * sig_14 - (r_122 * zm * sig_12) / fc_1 - (r_132 * zm * sig_11) / fc_1) +
                   (t_13 - t_23) *
                   (sig_8 * sig_18 + sig_7 * sig_17 - (r_112 * zm * sig_11) / fc_1 - (r_122 * zm * sig_10) / fc_1);
    double sig_1 =
            (sig_12 * sig_16 - sig_10 * sig_13) * (t_12 - t_22) - (sig_12 * sig_15 + sig_11 * sig_13) * (t_11 - t_21) +
            (sig_11 * sig_16 + sig_10 * sig_15) * (t_13 - t_23);
    double sig_6_sq = sig_6 * sig_6;

    d_Mya2_x = (sig_16 * sig_2) / (2 * sig_6) + (sig_18 * sig_5) / (2 * sig_6) -
               (sig_16 * sig_3 * sig_4) / (2 * sig_6_sq) - (sig_18 * sig_3 * sig_1) / (2 * sig_6_sq) -
               (r_112 * zm * sig_1) / (2 * fc_1 * sig_6);
    d_Mya2_y = (sig_15 * sig_2) / (2 * sig_6) + (sig_17 * sig_5) / (2 * sig_6) -
               (sig_15 * sig_3 * sig_4) / (2 * sig_6_sq) - (sig_17 * sig_3 * sig_1) / (2 * sig_6_sq) -
               (r_122 * zm * sig_1) / (2 * fc_1 * sig_6);
    d_Mya2_z = (sig_13 * sig_2) / (2 * sig_6) + (sig_14 * sig_5) / (2 * sig_6) -
               (sig_13 * sig_3 * sig_4) / (2 * sig_6_sq) - (sig_14 * sig_3 * sig_1) / (2 * sig_6_sq) -
               (r_132 * zm * sig_1) / (2 * fc_1 * sig_6);
}

void diff_Mxb2(const double& fc_1, const double& fc_2, // mean focal length camera 1 & 2
               const double& c1x, const double& c1y, // center frame camera 1
               const double& c2x, const double& c2y, // center frame camera 2
               const double& xMes_1, const double& yMes_1, // coordinates of the object in the frame of the first camera
               const double& xMes_2, const double& yMes_2, // coordinates of the object in the frame of the second camera
               const double& zm,                     // scale, distance plane of two frame
               const cv::Mat& Rmat_c1_to_w, const cv::Mat& tvec_c1_to_w,
               const cv::Mat& Rmat_c2_to_w, const cv::Mat& tvec_c2_to_w,
               double& d_Mxb2_x, double& d_Mxb2_y, double& d_Mxb2_z) {

    double t_11 = tvec_c1_to_w.at<double>(0), t_12 = tvec_c1_to_w.at<double>(1), t_13 = tvec_c1_to_w.at<double>(2),
            t_21 = tvec_c2_to_w.at<double>(0), t_22 = tvec_c2_to_w.at<double>(1), t_23 = tvec_c2_to_w.at<double>(2);
    double r_111 = Rmat_c1_to_w.at<double>(0, 0),
            r_112 = Rmat_c1_to_w.at<double>(0, 1),
            r_113 = Rmat_c1_to_w.at<double>(0, 2),
            r_121 = Rmat_c1_to_w.at<double>(1, 0),
            r_122 = Rmat_c1_to_w.at<double>(1, 1),
            r_123 = Rmat_c1_to_w.at<double>(1, 2),
            r_131 = Rmat_c1_to_w.at<double>(2, 0),
            r_132 = Rmat_c1_to_w.at<double>(2, 1),
            r_133 = Rmat_c1_to_w.at<double>(2, 2);
    double r_211 = Rmat_c2_to_w.at<double>(0, 0),
            r_212 = Rmat_c2_to_w.at<double>(0, 1),
            r_213 = Rmat_c2_to_w.at<double>(0, 2),
            r_221 = Rmat_c2_to_w.at<double>(1, 0),
            r_222 = Rmat_c2_to_w.at<double>(1, 1),
            r_223 = Rmat_c2_to_w.at<double>(1, 2),
            r_231 = Rmat_c2_to_w.at<double>(2, 0),
            r_232 = Rmat_c2_to_w.at<double>(2, 1),
            r_233 = Rmat_c2_to_w.at<double>(2, 2);

    double sig_18 = (r_211 * zm * (c2x - xMes_2)) / fc_2 - r_213 * zm + (r_212 * zm * (c2y - yMes_2)) / fc_2;
    double sig_17 = (r_221 * zm * (c2x - xMes_2)) / fc_2 - r_223 * zm + (r_222 * zm * (c2y - yMes_2)) / fc_2;
    double sig_16 = (r_111 * zm * (c1x - xMes_1)) / fc_1 - r_113 * zm + (r_112 * zm * (c1y - yMes_1)) / fc_1;
    double sig_15 = (r_121 * zm * (c1x - xMes_1)) / fc_1 - r_123 * zm + (r_122 * zm * (c1y - yMes_1)) / fc_1;
    double sig_14 = (r_231 * zm * (c2x - xMes_2)) / fc_2 - r_233 * zm + (r_232 * zm * (c2y - yMes_2)) / fc_2;
    double sig_13 = (r_131 * zm * (c1x - xMes_1)) / fc_1 - r_133 * zm + (r_132 * zm * (c1y - yMes_1)) / fc_1;
    double sig_12 = sig_16 * sig_17 - sig_15 * sig_18;
    double sig_11 = sig_16 * sig_14 - sig_13 * sig_18;
    double sig_10 = sig_15 * sig_14 - sig_13 * sig_17;
    double sig_9 = (r_221 * zm * sig_16) / fc_2 - (r_211 * zm * sig_15) / fc_2;
    double sig_8 = (r_231 * zm * sig_16) / fc_2 - (r_211 * zm * sig_13) / fc_2;
    double sig_7 = (r_231 * zm * sig_15) / fc_2 - (r_221 * zm * sig_13) / fc_2;
    double sig_6 = sig_12 * sig_12 + sig_11 * sig_11 + sig_10 * sig_10;
    double sig_5 =
            (sig_9 * sig_16 - sig_7 * sig_13) * (t_12 - t_22) - (sig_9 * sig_15 + sig_8 * sig_13) * (t_11 - t_21) +
            (sig_8 * sig_16 + sig_7 * sig_15) * (t_13 - t_23);
    double sig_4 =
            (sig_12 * sig_18 - sig_10 * sig_14) * (t_12 - t_22) - (sig_12 * sig_17 + sig_11 * sig_14) * (t_11 - t_21) +
            (sig_11 * sig_18 + sig_10 * sig_17) * (t_13 - t_23);
    double sig_3 = 2 * sig_12 * sig_9 + 2 * sig_11 * sig_8 + 2 * sig_10 * sig_7;
    double sig_2 = (t_12 - t_22) *
                   (sig_9 * sig_18 - sig_7 * sig_14 + (r_211 * zm * sig_12) / fc_2 - (r_231 * zm * sig_10) / fc_2) -
                   (t_11 - t_21) *
                   (sig_9 * sig_17 + sig_8 * sig_14 + (r_221 * zm * sig_12) / fc_2 + (r_231 * zm * sig_11) / fc_2) +
                   (t_13 - t_23) *
                   (sig_8 * sig_18 + sig_7 * sig_17 + (r_211 * zm * sig_11) / fc_2 + (r_221 * zm * sig_10) / fc_2);
    double sig_1 =
            (sig_12 * sig_16 - sig_10 * sig_13) * (t_12 - t_22) - (sig_12 * sig_15 + sig_11 * sig_13) * (t_11 - t_21) +
            (sig_11 * sig_16 + sig_10 * sig_15) * (t_13 - t_23);
    double sig_6_sq = sig_6 * sig_6;

    d_Mxb2_x = (sig_18 * sig_3 * sig_1) / (2 * sig_6_sq) - (sig_18 * sig_5) / (2 * sig_6) -
               (sig_16 * sig_2) / (2 * sig_6) + (sig_16 * sig_3 * sig_4) / (2 * sig_6_sq) -
               (r_211 * zm * sig_1) / (2 * fc_2 * sig_6);
    d_Mxb2_y = (sig_17 * sig_3 * sig_1) / (2 * sig_6_sq) - (sig_17 * sig_5) / (2 * sig_6) -
               (sig_15 * sig_2) / (2 * sig_6) + (sig_15 * sig_3 * sig_4) / (2 * sig_6_sq) -
               (r_221 * zm * sig_1) / (2 * fc_2 * sig_6);
    d_Mxb2_z = (sig_14 * sig_3 * sig_1) / (2 * sig_6_sq) - (sig_14 * sig_5) / (2 * sig_6) -
               (sig_13 * sig_2) / (2 * sig_6) + (sig_13 * sig_3 * sig_4) / (2 * sig_6_sq) -
               (r_231 * zm * sig_1) / (2 * fc_2 * sig_6);
}

void diff_Myb2(const double& fc_1, const double& fc_2, // mean focal length camera 1 & 2
               const double& c1x, const double& c1y, // center frame camera 1
               const double& c2x, const double& c2y, // center frame camera 2
               const double& xMes_1, const double& yMes_1, // coordinates of the object in the frame of the first camera
               const double& xMes_2, const double& yMes_2, // coordinates of the object in the frame of the second camera
               const double& zm,                     // scale, distance plane of two frame
               const cv::Mat& Rmat_c1_to_w, const cv::Mat& tvec_c1_to_w,
               const cv::Mat& Rmat_c2_to_w, const cv::Mat& tvec_c2_to_w,
               double& d_Myb2_x, double& d_Myb2_y, double& d_Myb2_z) {

    double t_11 = tvec_c1_to_w.at<double>(0), t_12 = tvec_c1_to_w.at<double>(1), t_13 = tvec_c1_to_w.at<double>(2),
            t_21 = tvec_c2_to_w.at<double>(0), t_22 = tvec_c2_to_w.at<double>(1), t_23 = tvec_c2_to_w.at<double>(2);
    double r_111 = Rmat_c1_to_w.at<double>(0, 0),
            r_112 = Rmat_c1_to_w.at<double>(0, 1),
            r_113 = Rmat_c1_to_w.at<double>(0, 2),
            r_121 = Rmat_c1_to_w.at<double>(1, 0),
            r_122 = Rmat_c1_to_w.at<double>(1, 1),
            r_123 = Rmat_c1_to_w.at<double>(1, 2),
            r_131 = Rmat_c1_to_w.at<double>(2, 0),
            r_132 = Rmat_c1_to_w.at<double>(2, 1),
            r_133 = Rmat_c1_to_w.at<double>(2, 2);
    double r_211 = Rmat_c2_to_w.at<double>(0, 0),
            r_212 = Rmat_c2_to_w.at<double>(0, 1),
            r_213 = Rmat_c2_to_w.at<double>(0, 2),
            r_221 = Rmat_c2_to_w.at<double>(1, 0),
            r_222 = Rmat_c2_to_w.at<double>(1, 1),
            r_223 = Rmat_c2_to_w.at<double>(1, 2),
            r_231 = Rmat_c2_to_w.at<double>(2, 0),
            r_232 = Rmat_c2_to_w.at<double>(2, 1),
            r_233 = Rmat_c2_to_w.at<double>(2, 2);

    double sig_18 = (r_211 * zm * (c2x - xMes_2)) / fc_2 - r_213 * zm + (r_212 * zm * (c2y - yMes_2)) / fc_2;
    double sig_17 = (r_221 * zm * (c2x - xMes_2)) / fc_2 - r_223 * zm + (r_222 * zm * (c2y - yMes_2)) / fc_2;
    double sig_16 = (r_111 * zm * (c1x - xMes_1)) / fc_1 - r_113 * zm + (r_112 * zm * (c1y - yMes_1)) / fc_1;
    double sig_15 = (r_121 * zm * (c1x - xMes_1)) / fc_1 - r_123 * zm + (r_122 * zm * (c1y - yMes_1)) / fc_1;
    double sig_14 = (r_231 * zm * (c2x - xMes_2)) / fc_2 - r_233 * zm + (r_232 * zm * (c2y - yMes_2)) / fc_2;
    double sig_13 = (r_131 * zm * (c1x - xMes_1)) / fc_1 - r_133 * zm + (r_132 * zm * (c1y - yMes_1)) / fc_1;
    double sig_12 = sig_16 * sig_17 - sig_15 * sig_18;
    double sig_11 = sig_16 * sig_14 - sig_13 * sig_18;
    double sig_10 = sig_15 * sig_14 - sig_13 * sig_17;
    double sig_9 = (r_222 * zm * sig_16) / fc_2 - (r_212 * zm * sig_15) / fc_2;
    double sig_8 = (r_232 * zm * sig_16) / fc_2 - (r_212 * zm * sig_13) / fc_2;
    double sig_7 = (r_232 * zm * sig_15) / fc_2 - (r_222 * zm * sig_13) / fc_2;
    double sig_6 = sig_12 * sig_12 + sig_11 * sig_11 + sig_10 * sig_10;
    double sig_5 =
            (sig_9 * sig_16 - sig_7 * sig_13) * (t_12 - t_22) - (sig_9 * sig_15 + sig_8 * sig_13) * (t_11 - t_21) +
            (sig_8 * sig_16 + sig_7 * sig_15) * (t_13 - t_23);
    double sig_4 =
            (sig_12 * sig_18 - sig_10 * sig_14) * (t_12 - t_22) - (sig_12 * sig_17 + sig_11 * sig_14) * (t_11 - t_21) +
            (sig_11 * sig_18 + sig_10 * sig_17) * (t_13 - t_23);
    double sig_3 = 2 * sig_12 * sig_9 + 2 * sig_11 * sig_8 + 2 * sig_10 * sig_7;
    double sig_2 = (t_12 - t_22) *
                   (sig_9 * sig_18 - sig_7 * sig_14 + (r_212 * zm * sig_12) / fc_2 - (r_232 * zm * sig_10) / fc_2) -
                   (t_11 - t_21) *
                   (sig_9 * sig_17 + sig_8 * sig_14 + (r_222 * zm * sig_12) / fc_2 + (r_232 * zm * sig_11) / fc_2) +
                   (t_13 - t_23) *
                   (sig_8 * sig_18 + sig_7 * sig_17 + (r_212 * zm * sig_11) / fc_2 + (r_222 * zm * sig_10) / fc_2);
    double sig_1 =
            (sig_12 * sig_16 - sig_10 * sig_13) * (t_12 - t_22) - (sig_12 * sig_15 + sig_11 * sig_13) * (t_11 - t_21) +
            (sig_11 * sig_16 + sig_10 * sig_15) * (t_13 - t_23);
    double sig_6_sq = sig_6 * sig_6;

    d_Myb2_x = (sig_18 * sig_3 * sig_1) / (2 * sig_6_sq) - (sig_18 * sig_5) / (2 * sig_6) -
               (sig_16 * sig_2) / (2 * sig_6) + (sig_16 * sig_3 * sig_4) / (2 * sig_6_sq) -
               (r_212 * zm * sig_1) / (2 * fc_2 * sig_6);
    d_Myb2_y = (sig_17 * sig_3 * sig_1) / (2 * sig_6_sq) - (sig_17 * sig_5) / (2 * sig_6) -
               (sig_15 * sig_2) / (2 * sig_6) + (sig_15 * sig_3 * sig_4) / (2 * sig_6_sq) -
               (r_222 * zm * sig_1) / (2 * fc_2 * sig_6);
    d_Myb2_z = (sig_14 * sig_3 * sig_1) / (2 * sig_6_sq) - (sig_14 * sig_5) / (2 * sig_6) -
               (sig_13 * sig_2) / (2 * sig_6) + (sig_13 * sig_3 * sig_4) / (2 * sig_6_sq) -
               (r_232 * zm * sig_1) / (2 * fc_2 * sig_6);
}

void compute_matrix_cam_to_world(const cv::Mat &Rmat_cam, const cv::Mat &tvec_cam,
                                 cv::Mat &Rmat_w_to_cam, cv::Mat &tvec_w_to_cam){
    Rmat_w_to_cam = Rmat_cam.t();
    tvec_w_to_cam = -Rmat_cam.t()*tvec_cam;
}

void calc_dispersion_measurement_3D_binocular(const double& std_x, const double& std_y,
                                              const double& fc_1, const double& fc_2, // mean focal length camera 1 & 2
                                              const double& c1x, const double& c1y, // center frame camera 1
                                              const double& c2x, const double& c2y, // center frame camera 2
                                              const double& xMes_1, const double& yMes_1, // coordinates of the object in the frame of the first camera
                                              const double& xMes_2, const double& yMes_2, // coordinates of the object in the frame of the second camera
                                              const double& zm,                     // scale, distance plane of two frame
                                              const cv::Mat& Rmat_w_to_c1, const cv::Mat& tvec_w_to_c1,
                                              const cv::Mat& Rmat_w_to_c2, const cv::Mat& tvec_w_to_c2,
                                              double&  dispersion_xp, double&  dispersion_yp, double&  dispersion_zp){

    cv::Mat Rmat_c1_to_w, tvec_c1_to_w, Rmat_c2_to_w, tvec_c2_to_w;
    compute_matrix_cam_to_world(Rmat_w_to_c1, tvec_w_to_c1, Rmat_c1_to_w, tvec_c1_to_w);
    compute_matrix_cam_to_world(Rmat_w_to_c2, tvec_w_to_c2, Rmat_c2_to_w, tvec_c2_to_w);

    double d_Mxa2_x, d_Mxa2_y, d_Mxa2_z,
            d_Mya2_x, d_Mya2_y, d_Mya2_z,
            d_Mxb2_x, d_Mxb2_y, d_Mxb2_z,
            d_Myb2_x, d_Myb2_y, d_Myb2_z;

    diff_Mxa2(fc_1, fc_2, c1x, c1y, c2x, c2y,
              xMes_1, yMes_1, xMes_2, yMes_2, zm,
              Rmat_c1_to_w, tvec_c1_to_w, Rmat_c2_to_w, tvec_c2_to_w,
              d_Mxa2_x, d_Mxa2_y, d_Mxa2_z);
    diff_Mya2(fc_1, fc_2, c1x, c1y, c2x, c2y,
              xMes_1, yMes_1, xMes_2, yMes_2, zm,
              Rmat_c1_to_w, tvec_c1_to_w, Rmat_c2_to_w, tvec_c2_to_w,
              d_Mya2_x, d_Mya2_y, d_Mya2_z);
    diff_Mxb2(fc_1, fc_2, c1x, c1y, c2x, c2y,
              xMes_1, yMes_1, xMes_2, yMes_2, zm,
              Rmat_c1_to_w, tvec_c1_to_w, Rmat_c2_to_w, tvec_c2_to_w,
              d_Mxb2_x, d_Mxb2_y, d_Mxb2_z);
    diff_Myb2(fc_1, fc_2, c1x, c1y, c2x, c2y,
              xMes_1, yMes_1, xMes_2, yMes_2, zm,
              Rmat_c1_to_w, tvec_c1_to_w, Rmat_c2_to_w, tvec_c2_to_w,
              d_Myb2_x, d_Myb2_y, d_Myb2_z);

    dispersion_xp = std_x * (d_Mxa2_x * d_Mxa2_x + d_Mxb2_x * d_Mxb2_x) + std_y * (d_Mya2_x * d_Mya2_x + d_Myb2_x * d_Myb2_x);
    dispersion_yp = std_x * (d_Mxa2_y * d_Mxa2_y + d_Mxb2_y * d_Mxb2_y) + std_y * (d_Mya2_y * d_Mya2_y + d_Myb2_y * d_Myb2_y);
    dispersion_zp = std_x * (d_Mxa2_z * d_Mxa2_z + d_Mxb2_z * d_Mxb2_z) + std_y * (d_Mya2_z * d_Mya2_z + d_Myb2_z * d_Myb2_z);
}