//
// Created by user on 05.08.24.
//
#include "coordinateTransformation.h"
#include <cmath>

// rotation matrix from the coordinate system of the template to the coordinate system of the right platform
double r11 = -0.437230524901427, r12 = -0.0479449585674082, r13 = -0.898070570190557,
        r21 = 0.898826925444235, r22 = -0.0573309419349090, r23 = -0.434538054942625,
        r31 = -0.0306533226728592, r32 = -0.997203311288538, r33 = 0.0681610575349883;
// coordinates of the origin of the template coordinate system in the coordinate system of the right platform
double t_x = -1830.57807769309, t_y = -3071.60280621011, t_z = 863.515954505610; //millimeter
// laser source coordinates in the turret coordinate system
double x_sl_CSPr = 0, y_sl_CSPr = -210, z_sl_CSPr = -26; //millimeter

double toRadians(double degrees) {
    return degrees * M_PI / 180.0;
}

void xy2XYZ(const double &x_frame, const double &y_frame,
            const double &focalLength_x, const double &focalLength_y,
            const double &u0, const double &v0, const double &distance,
            const double &x_cam0, const double &y_cam0, const double &z_cam0,
            const double &Az, const double &El,
            double &x_3d, double &y_3d, double &z_3d) {

    auto x_c = (x_frame - u0) / focalLength_x;
    auto y_c = (y_frame - v0) / focalLength_y;
    double Z_j = distance;// coordinate Z in SC camera = 1

    auto Az_r = -Az;
    auto El_r = El;
    x_3d = ((x_cam0 + Z_j) * cos(El_r)
            - (z_cam0 - y_c * Z_j) * sin(El_r)) * cos(Az_r)
           - (y_cam0 - x_c * Z_j) * sin(Az_r);
    y_3d = -((x_cam0 + Z_j) * cos(El_r)
             - (z_cam0 - y_c * Z_j) * sin(El_r)) * sin(Az_r)
           - (y_cam0 - x_c * Z_j) * cos(Az_r);
    z_3d = (x_cam0 + Z_j) * sin(El_r)
           + (z_cam0 - y_c * Z_j) * cos(El_r);
}

void xy2XYZ(const TDataFrame &data_frame, const TDataCam &data_cam,
            const double &x_frame, const double &y_frame,
            const double &f_x, const double &f_y, const double &c_x, const double &c_y,
            double &Xp, double &Yp, double &Zp) {

    double distance_to_object = 1.0, focalLength_x = f_x, focalLength_y = f_y;

    float Az = data_frame.Az;
    float El = data_frame.El;
    double u0 = c_x;//data_frame.width / 2;
    double v0 = c_y;//data_frame.height / 2;

    float Xcam = data_cam.Xcam;
    float Ycam = data_cam.Ycam;
    float Zcam = data_cam.Zcam;

    xy2XYZ(x_frame, y_frame,
           focalLength_x, focalLength_y,
           u0, v0, distance_to_object,
           Xcam, Ycam, Zcam,
           Az, El, Xp, Yp, Zp);
}

void XYZ2xy(const double &x_3d, const double &y_3d, const double &z_3d,
            const double &focalLength_x, const double &focalLength_y,
            const double &u0, const double &v0,
            const double &x_cam0, const double &y_cam0, const double &z_cam0,
            const double &Az, const double &El,
            double &x_frame, double &y_frame) {
    double x_tm, y_tm, z_tm, tmp;
    tmp = x_3d * cos(Az) + y_3d * sin(Az);
    x_tm = (tmp) * cos(El) + z_3d * sin(El);
    y_tm = x_3d * sin(Az) - y_3d * cos(Az);
    z_tm = -(tmp) * sin(El) + z_3d * cos(El);

    x_frame = focalLength_x * (-y_tm + y_cam0) / (x_tm - x_cam0) + u0;
    y_frame = focalLength_y * (-z_tm + z_cam0) / (x_tm - x_cam0) + v0;
}

void extrapolation_coor(const double &x0_hat, const double &Vx_hat, const double &ax_hat, const double &dt,
                        double &Xp, double &Vx, double &ax) {
    Xp = x0_hat + Vx_hat * dt + ax_hat * dt * dt;
    Vx = Vx_hat + 2 * ax_hat * dt;
    ax = 2 * ax_hat;
}

void extrapolationAzEl_V_a(const Trajectory &trajectory, const DateTime &time_extra,
                           double &Az, double &El,
                           double &VAz, double &VEl,
                           double &aAz, double &aEl) {

    double xp, yp, zp, Vx, Vy, Vz, ax, ay, az, dist, d2, dxy2;
    double dt = std::chrono::duration<double>(time_extra - trajectory.time0).count();
    // extrapolation of 3D pseudo coordinates
    extrapolation_coor(trajectory.x0_hat, trajectory.Vx_hat, trajectory.ax_hat, dt,
                       xp, Vx, ax);
    extrapolation_coor(trajectory.y0_hat, trajectory.Vy_hat, trajectory.ay_hat, dt,
                       yp, Vy, ay);
    extrapolation_coor(trajectory.z0_hat, trajectory.Vz_hat, trajectory.az_hat, dt,
                       zp, Vz, az);
    // calculation of predicted spherical coordinates
    d2 = xp * xp + yp * yp + zp * zp;
    dxy2 = xp * xp + yp * yp;
    dist = sqrt(d2);
    Az = atan2(yp, xp);
    El = asin(zp / dist);
    // calculation of predicted speed in spherical coordinates
    VAz = (xp * Vy - yp * Vx) / dxy2;
    VEl = (dxy2 * Vz - zp * (xp * Vx + yp * Vy)) / (sqrt(dxy2) * d2);
    // calculation of predicted acceleration in spherical coordinates
    aAz = (xp * yp * (2 * Vx * Vx + yp * ay - 2 * Vy * Vy) - xp * xp * (yp * ax + 2 * Vx * Vy) +
           yp * yp * (2 * Vx * Vy - yp * ax) + xp * xp * xp * ay) / (dxy2 * dxy2);
    aEl = (4 * zp * pow((zp * (xp * Vx + yp * Vy) - dxy2 * Vz), 2) -
           dxy2 * (8 * Vz * d2 * (xp * Vx + yp * Vy + zp * Vz) -
                   12 * zp * pow(xp * Vx + yp * Vy + zp * Vz, 2) +
                   4 * zp * d2 * (xp * ax + Vx * Vx + yp * ay + Vy * Vy + zp * az + Vz * Vz) - 4 * az * d2 * d2)) /
          (4 * pow(sqrt(dxy2), 3) * d2 * d2);
}

void extrapolationAzEl_4_binocular(const Trajectory &trajectory, const DateTime &time_extra,
                                   double &Az, double &El) {
    double xp, yp, zp, Vx, Vy, Vz, ax, ay, az;
    double dt = std::chrono::duration<double>(time_extra - trajectory.time0).count();
    // extrapolation of 3D pseudo coordinates
    extrapolation_coor(trajectory.x0_hat, trajectory.Vx_hat, trajectory.ax_hat, dt,
                       xp, Vx, ax);
    extrapolation_coor(trajectory.y0_hat, trajectory.Vy_hat, trajectory.ay_hat, dt,
                       yp, Vy, ay);
    extrapolation_coor(trajectory.z0_hat, trajectory.Vz_hat, trajectory.az_hat, dt,
                       zp, Vz, az);

    double x_t_CSPr = r11 * xp + r12 * yp + r13 * zp + t_x,
            y_t_CSPr = r21 * xp + r22 * yp + r23 * zp + t_y,
            z_t_CSPr = r31 * xp + r32 * yp + r33 * zp + t_z;

    double sq_x_t_CSPr = x_t_CSPr * x_t_CSPr,
            sq_y_t_CSPr = y_t_CSPr * y_t_CSPr,
            sq_z_t_CSPr = z_t_CSPr * z_t_CSPr,
            sq_y_sl_CSPr = y_sl_CSPr * y_sl_CSPr,
            sq_z_sl_CSPr = z_sl_CSPr * z_sl_CSPr;

    double part_1 = sqrt(sq_x_t_CSPr + sq_y_t_CSPr + sq_z_t_CSPr);
    double part_2 = sqrt(sq_x_t_CSPr + sq_y_t_CSPr + sq_z_t_CSPr - sq_y_sl_CSPr - sq_z_sl_CSPr);
    double part_3 = sqrt(sq_x_t_CSPr + sq_y_t_CSPr + sq_z_t_CSPr - sq_z_sl_CSPr);
    double part_4 = sqrt(sq_x_t_CSPr + sq_y_t_CSPr);

    double part_x_atan = x_t_CSPr * part_4 * part_3 / part_1 + x_t_CSPr * z_t_CSPr * z_sl_CSPr / part_1 +
                         y_t_CSPr * y_sl_CSPr * part_1 / part_2;
    double part_y_atan = y_t_CSPr * part_4 * part_3 / part_1 + y_t_CSPr * z_t_CSPr * z_sl_CSPr / part_1 -
                         x_t_CSPr * y_sl_CSPr * part_1 / part_2;

    double az_ptg_CSPr = atan2(part_y_atan, part_x_atan);
    double el_ptg_CSPr = -asin((z_sl_CSPr * part_4 - z_t_CSPr * part_3) / (part_1 * part_1 * part_3 / part_2));

    Az = -az_ptg_CSPr; // az_ptg_CSPl
    El = el_ptg_CSPr; // el_ptg_CSPl
}

double velocity_Az_binocular(const double &x_0, const double &y_0, const double &z_0,
                             const double &Vx, const double &Vy, const double &Vz,
                             const double &ax, const double &ay, const double &az,
                             const double &t) {
    double sig_14 = z_0 + Vz * t + az * t;
    double sig_13 = x_0 + Vx * t + ax * t;
    double sig_12 = y_0 + Vy * t + ay * t;
    double sq_sig_13 = sig_13 * sig_13,
            sq_sig_12 = sig_12 * sig_12,
            sq_sig_14 = sig_14 * sig_14,
            sq_y_sl_CSPr = y_sl_CSPr * y_sl_CSPr,
            sq_z_sl_CSPr = z_sl_CSPr * z_sl_CSPr,
            cube_sig_12 = sig_12 * sig_12 * sig_12;
    double sig_11 = sq_sig_13 + sq_sig_12 + sq_sig_14;
    double sig_10 = sq_sig_13 + sq_sig_12 + sq_sig_14 - sq_y_sl_CSPr - sq_z_sl_CSPr;
    double sig_9 = 2 * (Vx + ax) * sig_13;
    double sig_8 = 2 * (Vy + ay) * sig_12;
    double sig_7 = sqrt(sq_sig_13 + sq_sig_12);
    double sig_6 = sqrt(sq_sig_13 + sq_sig_12 + sq_sig_14 - sq_z_sl_CSPr);
    double sig_5 = 2 * sqrt(sig_11 * sig_11 * sig_11);
    double sig_4 = 2 * sqrt(sig_10 * sig_10 * sig_10);
    double sig_3 = sig_9 + sig_8 + 2 * (Vz + az) * sig_14;
    double sig_2 = (sig_7 * sig_12 * sig_6) / sqrt(sig_11)
                   - (y_sl_CSPr * sqrt(sig_11) * sig_13) / sqrt(sig_10)
                   + (z_sl_CSPr * sig_12 * sig_14) / sqrt(sig_11);
    double sig_1 = sig_7 * sig_13 * sig_6 + z_sl_CSPr * sig_13 * sig_14 + (y_sl_CSPr * cube_sig_12) / sqrt(sig_10)
                   + (y_sl_CSPr * sq_sig_13 * sig_12) / sqrt(sig_10) + (y_sl_CSPr * sig_12 * sq_sig_14) / sqrt(sig_10);
    double sq_sig_1 = sig_1 * sig_1;
    double Vaz = ((sqrt(sig_11) * (((Vy + ay) * sig_7 * sig_6) / sqrt(sig_11)
                                   - (y_sl_CSPr * (Vx + ax) * sqrt(sig_11)) / sqrt(sig_10)
                                   + (z_sl_CSPr * (Vz + az) * sig_12) / sqrt(sig_11) +
                                   (z_sl_CSPr * (Vy + ay) * sig_14) / sqrt(sig_11)
                                   + (sig_7 * sig_12 * sig_3) / (2 * sqrt(sig_11) * sig_6)
                                   - (sig_7 * sig_12 * sig_3 * sig_6) / sig_5 -
                                   (z_sl_CSPr * sig_12 * sig_14 * sig_3) / sig_5
                                   + ((sig_9 + sig_8) * sig_12 * sig_6) / (2 * sig_7 * sqrt(sig_11))
                                   - (y_sl_CSPr * sig_13 * sig_3) / (2 * sqrt(sig_11) * sqrt(sig_10))
                                   + (y_sl_CSPr * sqrt(sig_11) * sig_13 * sig_3) / sig_4)) / sig_1
                  - (sqrt(sig_11) * sig_2 *
                     ((Vx + ax) * sig_7 * sig_6 + z_sl_CSPr * (Vz + az) * sig_13 + z_sl_CSPr * (Vx + ax) * sig_14
                      + (y_sl_CSPr * (Vy + ay) * sq_sig_13) / sqrt(sig_10)
                      + (3 * y_sl_CSPr * (Vy + ay) * sq_sig_12) / sqrt(sig_10) +
                      (y_sl_CSPr * (Vy + ay) * sq_sig_14) / sqrt(sig_10)
                      + (sig_7 * sig_13 * sig_3) / (2 * sig_6)
                      - (y_sl_CSPr * cube_sig_12 * sig_3) / sig_4 + ((sig_9 + sig_8) * sig_13 * sig_6) / (2 * sig_7)
                      - (y_sl_CSPr * sq_sig_13 * sig_12 * sig_3) / sig_4
                      - (y_sl_CSPr * sig_12 * sq_sig_14 * sig_3) / sig_4
                      + (2 * y_sl_CSPr * (Vx + ax) * sig_13 * sig_12) / sqrt(sig_10)
                      + (2 * y_sl_CSPr * (Vz + az) * sig_12 * sig_14) / sqrt(sig_10))) / sq_sig_1
                  + (sig_3 * sig_2) / (2 * sqrt(sig_11) * sig_1)) / ((sig_11 * sig_2 * sig_2) / sq_sig_1 + 1);
    return Vaz;
}

double velocity_El_binocular(const double &x_0, const double &y_0, const double &z_0,
                             const double &Vx, const double &Vy, const double &Vz,
                             const double &ax, const double &ay, const double &az,
                             const double &t) {
    double sig_25 = z_0 + Vz * t + az * t;
    double sig_24 = x_0 + Vx * t + ax * t;
    double sig_23 = y_0 + Vy * t + ay * t;
    double sq_sig_25 = sig_25 * sig_25,
            sq_sig_24 = sig_24 * sig_24,
            sq_sig_23 = sig_23 * sig_23,
            sq_y_sl_CSPr = y_sl_CSPr * y_sl_CSPr,
            sq_z_sl_CSPr = z_sl_CSPr * z_sl_CSPr,
            sq3_sig_24 = pow(sig_24, 3.0),
            sq5_sig_24 = pow(sig_24, 5.0);
    double sig_22 = sq_sig_24 + sq_sig_23 + sq_sig_25 - sq_y_sl_CSPr - sq_z_sl_CSPr;
    double sig_21 = 2 * (Vx + ax) * sig_24;
    double sig_20 = 2 * (Vy + ay) * sig_23;
    double sig_19 = sq_sig_24 + sq_sig_23 + sq_sig_25;
    double sig_18 = sq_sig_24 + sq_sig_23 + sq_sig_25 - sq_z_sl_CSPr;
    double sig_17 = sqrt(sq_sig_24 + sq_sig_23);
    double sig_16 = sqrt(sq_y_sl_CSPr / sig_22 + 1);
    double sig_15 = sig_21 + sig_20 + 2 * (Vz + az) * sig_25;
    double sig_14 = sq_sig_23 / sq_sig_24 + 1;
    double sig_13 = sqrt(sig_18 / sig_22) * sqrt(sig_19);
    double sig_12 = sig_17 * sig_24 * sqrt(sig_18) + z_sl_CSPr * sig_24 * sig_25
                    + (y_sl_CSPr * sq_sig_23 * sig_23) / sqrt(sig_22) + (y_sl_CSPr * sq_sig_24 * sig_23) / sqrt(sig_22)
                    + (y_sl_CSPr * sig_23 * sq_sig_25) / sqrt(sig_22);
    double sq_sig_12 = sig_12 * sig_12;
    double sig_11 = z_sl_CSPr * sig_17 * sig_16 * sig_15 * sig_22;
    double sig_10 = sig_14 * sq_sig_24 * sig_18;
    double sig_9 = sig_14 * sig_19 * sq_sig_24 * sig_18;
    double sig_8 = (sig_17 * sig_23 * sqrt(sig_18)) / sqrt(sig_19)
                   - (y_sl_CSPr * sqrt(sig_19) * sig_24) / sqrt(sig_22)
                   + (z_sl_CSPr * sig_23 * sig_25) / sqrt(sig_19);
    double sig_7 = (sig_25 * sqrt(sig_18)) / sig_13 - (z_sl_CSPr * sig_17 * sig_16 * sig_22) / (sqrt(sig_19) * sig_18);
    double sig_6 = sig_15 * sq_sig_12 * sig_22;
    double sig_5 = 2 * sqrt(sig_19 * sig_19 * sig_19);
    double sig_4 = 2 * sqrt(sig_22 * sig_22 * sig_22);
    double p_sig_19 = pow(sig_19, 1.5),
            sq_sig_19 = sig_19 * sig_19,
            sq_sig_8 = sig_8 * sig_8,
            sq_sig_7 = sig_7 * sig_7,
            sq_sig_18 = sig_18 * sig_18,
            sq_sig_14 = sig_14 * sig_14;
    double sig_3 = ((Vz + az) * sqrt(sig_18)) / sig_13 - ((sig_15 / sig_22
                                                           - (sig_15 * sig_18) / pow(sig_22, 2.0)) * sig_25 *
                                                          sqrt(sig_18)) / (2 * pow(sig_18 / sig_22, 1.5) * sqrt(sig_19))
                   + (sig_25 * sig_15) / (2 * sqrt(sig_18 / sig_22) * sqrt(sig_19) * sqrt(sig_18))
                   - (sig_25 * sig_15 * sqrt(sig_18)) / (2 * sqrt(sig_18 / sig_22) * p_sig_19)
                   - (z_sl_CSPr * sig_17 * sig_16 * sig_15) / (sqrt(sig_19) * sig_18)
                   - (z_sl_CSPr * sig_16 * (sig_21 + sig_20) * sig_22) / (2 * sig_17 * sqrt(sig_19) * sig_18)
                   + sig_11 / (sqrt(sig_19) * sq_sig_18) + sig_11 / (2 * p_sig_19 * sig_18)
                   + (sq_y_sl_CSPr * z_sl_CSPr * sig_17 * sig_15) / (2 * sig_16 * sqrt(sig_19) * sig_18 * sig_22);
    double sig_2 = sq_sig_7 + (sq_sig_8 * sig_22) / sig_10 + (sq_sig_12 * sig_22) / sig_9;
    double sig_1 = Vy * x_0 - Vx * y_0 + ay * x_0 - ax * y_0;

    double Vel =
            (sig_3 / sqrt(sig_2) + (sig_7 * ((2 * (Vx + ax) * sq_sig_8 * sig_22) / (sig_14 * sq3_sig_24 * sig_18)
                                             - (sig_15 * sq_sig_8) / sig_10 - 2 * sig_7 * sig_3 -
                                             (2 * sig_8 * sig_22 * (((Vy + ay) * sig_17 * sqrt(sig_18)) / sqrt(sig_19)
                                                                    - (y_sl_CSPr * (Vx + ax) * sqrt(sig_19)) /
                                                                      sqrt(sig_22)
                                                                    + (z_sl_CSPr * (Vz + az) * sig_23) / sqrt(sig_19)
                                                                    + (z_sl_CSPr * (Vy + ay) * sig_25) / sqrt(sig_19)
                                                                    + (sig_17 * sig_23 * sig_15) /
                                                                      (2 * sqrt(sig_19) * sqrt(sig_18))
                                                                    -
                                                                    (sig_17 * sig_23 * sig_15 * sqrt(sig_18)) / sig_5 -
                                                                    (z_sl_CSPr * sig_23 * sig_25 * sig_15) / sig_5
                                                                    + ((sig_21 + sig_20) * sig_23 * sqrt(sig_18)) /
                                                                      (2 * sig_17 * sqrt(sig_19))
                                                                    - (y_sl_CSPr * sig_24 * sig_15) /
                                                                      (2 * sqrt(sig_19) * sqrt(sig_22))
                                                                    + (y_sl_CSPr * sqrt(sig_19) * sig_24 * sig_15) /
                                                                      sig_4)) / sig_10 - (sig_15 * sq_sig_12) / sig_9
                                             + (sig_15 * sq_sig_8 * sig_22) / (sig_14 * sq_sig_24 * sq_sig_18)
                                             + (2 * (Vx + ax) * sq_sig_12 * sig_22) /
                                               (sig_14 * sig_19 * sq3_sig_24 * sig_18)
                                             + sig_6 / (sig_14 * sig_19 * sq_sig_24 * sq_sig_18) +
                                             sig_6 / (sig_14 * sq_sig_19 * sq_sig_24 * sig_18)
                                             + (2 * sig_23 * sq_sig_8 * sig_1 * sig_22) /
                                               (sq_sig_14 * sq5_sig_24 * sig_18)
                                             - (2 * sig_12 * sig_22 *
                                                ((Vx + ax) * sig_17 * sqrt(sig_18) + z_sl_CSPr * (Vz + az) * sig_24
                                                 + z_sl_CSPr * (Vx + ax) * sig_25 +
                                                 (y_sl_CSPr * (Vy + ay) * sq_sig_24) / sqrt(sig_22)
                                                 + (3 * y_sl_CSPr * (Vy + ay) * sq_sig_23) / sqrt(sig_22)
                                                 + (y_sl_CSPr * (Vy + ay) * sq_sig_25) / sqrt(sig_22)
                                                 + (sig_17 * sig_24 * sig_15) / (2 * sqrt(sig_18)) -
                                                 (y_sl_CSPr * sq_sig_23 * sig_23 * sig_15) / sig_4
                                                 + ((sig_21 + sig_20) * sig_24 * sqrt(sig_18)) / (2 * sig_17)
                                                 - (y_sl_CSPr * sq_sig_24 * sig_23 * sig_15) / sig_4
                                                 - (y_sl_CSPr * sig_23 * sq_sig_25 * sig_15) / sig_4
                                                 + (2 * y_sl_CSPr * (Vx + ax) * sig_24 * sig_23) / sqrt(sig_22)
                                                 + (2 * y_sl_CSPr * (Vz + az) * sig_23 * sig_25) / sqrt(sig_22))) /
                                               sig_9
                                             + (2 * sig_23 * sig_1 * sq_sig_12 * sig_22)
                                               / (sq_sig_14 * sig_19 * sq5_sig_24 * sig_18)))
                                   / (2 * pow(sig_2, 1.5))) / sqrt(1 - sq_sig_7 / sig_2);
    return Vel;
}

void extrapolationAzEl_V_4_binocular(const Trajectory &trajectory, const DateTime &time_extra,
                                     double &Az, double &El,
                                     double &VAz, double &VEl) {
    double xp, yp, zp, Vx, Vy, Vz, ax, ay, az;
    double dt = std::chrono::duration<double>(time_extra - trajectory.time0).count();
    // extrapolation of 3D pseudo coordinates
    extrapolation_coor(trajectory.x0_hat, trajectory.Vx_hat, trajectory.ax_hat, dt,
                       xp, Vx, ax);
    extrapolation_coor(trajectory.y0_hat, trajectory.Vy_hat, trajectory.ay_hat, dt,
                       yp, Vy, ay);
    extrapolation_coor(trajectory.z0_hat, trajectory.Vz_hat, trajectory.az_hat, dt,
                       zp, Vz, az);

    double x_t_CSPr = r11 * xp + r12 * yp + r13 * zp + t_x,
            y_t_CSPr = r21 * xp + r22 * yp + r23 * zp + t_y,
            z_t_CSPr = r31 * xp + r32 * yp + r33 * zp + t_z;

    double sq_x_t_CSPr = x_t_CSPr * x_t_CSPr,
            sq_y_t_CSPr = y_t_CSPr * y_t_CSPr,
            sq_z_t_CSPr = z_t_CSPr * z_t_CSPr,
            sq_y_sl_CSPr = y_sl_CSPr * y_sl_CSPr,
            sq_z_sl_CSPr = z_sl_CSPr * z_sl_CSPr;

    double part_1 = sqrt(sq_x_t_CSPr + sq_y_t_CSPr + sq_z_t_CSPr);
    double part_2 = sqrt(sq_x_t_CSPr + sq_y_t_CSPr + sq_z_t_CSPr - sq_y_sl_CSPr - sq_z_sl_CSPr);
    double part_3 = sqrt(sq_x_t_CSPr + sq_y_t_CSPr + sq_z_t_CSPr - sq_z_sl_CSPr);
    double part_4 = sqrt(sq_x_t_CSPr + sq_y_t_CSPr);

    double part_x_atan = x_t_CSPr * part_4 * part_3 / part_1 + x_t_CSPr * z_t_CSPr * z_sl_CSPr / part_1 +
                         y_t_CSPr * y_sl_CSPr * part_1 / part_2;
    double part_y_atan = y_t_CSPr * part_4 * part_3 / part_1 + y_t_CSPr * z_t_CSPr * z_sl_CSPr / part_1 -
                         x_t_CSPr * y_sl_CSPr * part_1 / part_2;

    double az_ptg_CSPr = atan2(part_y_atan, part_x_atan);
    double el_ptg_CSPr = -asin((z_sl_CSPr * part_4 - z_t_CSPr * part_3) / (part_1 * part_1 * part_3 / part_2));

    Az = -az_ptg_CSPr; // az_ptg_CSPl
    El = el_ptg_CSPr; // el_ptg_CSPl

    VAz = -velocity_Az_binocular(xp, yp, zp, Vx, Vy, Vz, ax, ay, az, dt);
    VEl = velocity_El_binocular(xp, yp, zp, Vx, Vy, Vz, ax, ay, az, dt);
}

void transformation_CSSam_to_CSPr(const double &x0_hat, const double &Vx_hat, const double &ax_hat,
                                  const double &y0_hat, const double &Vy_hat, const double &ay_hat,
                                  const double &z0_hat, const double &Vz_hat, const double &az_hat,
                                  double &x_0, double &Vx, double &ax,
                                  double &y_0, double &Vy, double &ay,
                                  double &z_0, double &Vz, double &az) {
    x_0 = r11 * x0_hat + r12 * y0_hat + r13 * z0_hat + t_x,
    y_0 = r21 * x0_hat + r22 * y0_hat + r23 * z0_hat + t_y,
    z_0 = r31 * x0_hat + r32 * y0_hat + r33 * z0_hat + t_z;
    Vx = r11 * Vx_hat + r12 * Vy_hat + r13 * Vz_hat;
    Vy = r21 * Vx_hat + r22 * Vy_hat + r23 * Vz_hat;
    Vz = r31 * Vx_hat + r32 * Vy_hat + r33 * Vz_hat;
    ax = r11 * ax_hat + r12 * ay_hat + r13 * az_hat;
    ay = r21 * ax_hat + r22 * ay_hat + r23 * az_hat;
    az = r31 * ax_hat + r32 * ay_hat + r33 * az_hat;
}

double Az_CSPr(const double &xe, const double &ye, const double &ze) {
    double sig_6 = ze;
    double sig_5 = xe;
    double sig_4 = ye;
    double sq_sig_4 = sig_4 * sig_4,
            sq_sig_5 = sig_5 * sig_5,
            sq_sig_6 = sig_6 * sig_6,
            sq_y_sl_CSPr = y_sl_CSPr * y_sl_CSPr,
            sq_z_sl_CSPr = z_sl_CSPr * z_sl_CSPr;
    double sig_3 = sqrt(sq_sig_5 - sq_z_sl_CSPr + sq_sig_4 + sq_sig_6);
    double sig_2 = sqrt(sq_sig_5 + sq_sig_4 + sq_sig_6);
    double sig_1 = sqrt(sq_sig_5 - sq_z_sl_CSPr - sq_y_sl_CSPr + sq_sig_4 + sq_sig_6);

    double Az = atan((sig_2 * ((sqrt(sq_sig_5 + sq_sig_4) * sig_4 * sig_3) / sig_2
                               - (y_sl_CSPr * sig_2 * sig_5) / sig_1
                               + (z_sl_CSPr * sig_4 * sig_6) / sig_2)) /
                     (z_sl_CSPr * sig_5 * sig_6 + sqrt(sq_sig_5 + sq_sig_4) * sig_5 * sig_3
                      + (y_sl_CSPr * sig_4 * sq_sig_4) / sig_1 + (y_sl_CSPr * sq_sig_5 * sig_4) / sig_1 +
                      (y_sl_CSPr * sig_4 * sq_sig_6) / sig_1));
    return Az;
}

double El_CSPr(const double &xe, const double &ye, const double &ze) {
    double sig_9 = xe;
    double sig_8 = ze;
    double sig_7 = ye;
    double sq_sig_9 = sig_9 * sig_9,
            sq_sig_8 = sig_8 * sig_8,
            sq_sig_7 = sig_7 * sig_7,
            sq_y_sl_CSPr = y_sl_CSPr * y_sl_CSPr,
            sq_z_sl_CSPr = z_sl_CSPr * z_sl_CSPr;
    double sig_6 = sq_sig_9 + sq_sig_7 + sq_sig_8;
    double sig_5 = sqrt(sq_sig_9 + sq_sig_7);
    double sig_4 = sq_sig_9 - sq_z_sl_CSPr + sq_sig_7 + sq_sig_8;
    double sig_3 = sq_sig_9 - sq_z_sl_CSPr - sq_y_sl_CSPr + sq_sig_7 + sq_sig_8;
    double sig_2 = sq_sig_7 / sq_sig_9 + 1;
    double sig_1 = (sig_8 * sqrt(sig_4)) / (sqrt(sig_4 / sig_3) * sqrt(sig_6))
                   - (z_sl_CSPr * sig_5 * sqrt(sq_y_sl_CSPr / sig_3 + 1) * sig_3) / (sqrt(sig_6) * sig_4);

    double El = asin(sig_1 / sqrt(pow(sig_1, 2.0) + (pow((sig_5 * sig_7 * sqrt(sig_4)) / sqrt(sig_6)
                                                         - (y_sl_CSPr * sqrt(sig_6) * sig_9) / sqrt(sig_3)
                                                         + (z_sl_CSPr * sig_7 * sig_8) / sqrt(sig_6), 2.0) * sig_3) /
                                                    (sig_2 * sq_sig_9 * sig_4)
                                  + (pow(z_sl_CSPr * sig_9 * sig_8 + sig_5 * sig_9 * sqrt(sig_4) +
                                         (y_sl_CSPr * sq_sig_7 * sig_7) / sqrt(sig_3)
                                         + (y_sl_CSPr * sq_sig_9 * sig_7) / sqrt(sig_3)
                                         + (y_sl_CSPr * sig_7 * sq_sig_8) / sqrt(sig_3), 2.0) * sig_3) /
                                    (sig_2 * sig_6 * sq_sig_9 * sig_4)));
    return El;
}

double Vaz_CSPr(const double &xe, const double &ye, const double &ze,
                const double &Vx, const double &Vy, const double &Vz,
                const double &ax, const double &ay, const double &az, const double &dt) {
    double sig_16 = ze;
    double sig_15 = xe;
    double sig_14 = ye;
    double sq_sig_15 = sig_15 * sig_15,
            sq_sig_14 = sig_14 * sig_14,
            sq_sig_16 = sig_16 * sig_16,
            sq_y_sl_CSPr = y_sl_CSPr * y_sl_CSPr,
            sq_z_sl_CSPr = z_sl_CSPr * z_sl_CSPr,
            sq3_sig_14 = sig_14 * sq_sig_14;
    double sig_13 = sq_sig_15 + sq_sig_14 + sq_sig_16;
    double sig_12 = Vx + 2 * ax * dt;
    double sig_11 = Vy + 2 * ay * dt;
    double sig_10 = sq_sig_15 - sq_z_sl_CSPr - sq_y_sl_CSPr + sq_sig_14 + sq_sig_16;
    double sig_9 = Vz + 2 * az * dt;
    double sig_8 = sqrt(sq_sig_15 + sq_sig_14);
    double sig_7 = sqrt(sq_sig_15 - sq_z_sl_CSPr + sq_sig_14 + sq_sig_16);
    double sig_6 = 2 * pow(sig_13, 1.5);
    double sig_5 = 2 * sig_12 * sig_15 + 2 * sig_11 * sig_14;
    double sig_4 = 2 * pow(sig_10, 1.5);
    double sig_3 = 2 * sig_12 * sig_15 + 2 * sig_11 * sig_14 + 2 * sig_9 * sig_16;
    double sig_2 = (sig_8 * sig_14 * sig_7) / sqrt(sig_13)
                   - (y_sl_CSPr * sqrt(sig_13) * sig_15) / sqrt(sig_10)
                   + (z_sl_CSPr * sig_14 * sig_16) / sqrt(sig_13);
    double sig_1 = z_sl_CSPr * sig_15 * sig_16 + sig_8 * sig_15 * sig_7
                   + (y_sl_CSPr * sq3_sig_14) / sqrt(sig_10)
                   + (y_sl_CSPr * sq_sig_15 * sig_14) / sqrt(sig_10)
                   + (y_sl_CSPr * sig_14 * sq_sig_16) / sqrt(sig_10);
    double sq_sig_1 = sig_1 * sig_1;
    double Vaz = ((sqrt(sig_13) * ((sig_8 * sig_11 * sig_7) / sqrt(sig_13)
                                   - (y_sl_CSPr * sig_12 * sqrt(sig_13)) / sqrt(sig_10)
                                   + (z_sl_CSPr * sig_9 * sig_14) / sqrt(sig_13)
                                   + (z_sl_CSPr * sig_11 * sig_16) / sqrt(sig_13)
                                   + (sig_8 * sig_3 * sig_14) / (2 * sqrt(sig_13) * sig_7)
                                   - (sig_8 * sig_3 * sig_14 * sig_7) / sig_6
                                   - (y_sl_CSPr * sig_3 * sig_15) / (2 * sqrt(sig_13) * sqrt(sig_10))
                                   + (y_sl_CSPr * sig_3 * sqrt(sig_13) * sig_15) / sig_4
                                   + (sig_5 * sig_14 * sig_7) / (2 * sig_8 * sqrt(sig_13))
                                   - (z_sl_CSPr * sig_3 * sig_14 * sig_16) / sig_6)) / sig_1
                  + (sig_3 * sig_2) / (2 * sqrt(sig_13) * sig_1)
                  - (sqrt(sig_13) * sig_2 *
                     (z_sl_CSPr * sig_9 * sig_15 + z_sl_CSPr * sig_12 * sig_16 + sig_8 * sig_12 * sig_7
                      + (sig_8 * sig_3 * sig_15) / (2 * sig_7) + (sig_5 * sig_15 * sig_7) / (2 * sig_8)
                      + (y_sl_CSPr * sig_11 * sq_sig_15) / sqrt(sig_10)
                      + (3 * y_sl_CSPr * sig_11 * sq_sig_14) / sqrt(sig_10)
                      + (y_sl_CSPr * sig_11 * sq_sig_16) / sqrt(sig_10)
                      - (y_sl_CSPr * sig_3 * sq3_sig_14) / sig_4 +
                      (2 * y_sl_CSPr * sig_12 * sig_15 * sig_14) / sqrt(sig_10)
                      + (2 * y_sl_CSPr * sig_9 * sig_14 * sig_16) / sqrt(sig_10)
                      - (y_sl_CSPr * sig_3 * sq_sig_15 * sig_14) / sig_4
                      - (y_sl_CSPr * sig_3 * sig_14 * sq_sig_16) / sig_4)) / sq_sig_1) /
                 ((sig_13 * sig_2 * sig_2) / sq_sig_1 + 1);
    return Vaz;
}

double Vel_CSPr(const double &xe, const double &ye, const double &ze,
                const double &Vx, const double &Vy, const double &Vz,
                const double &ax, const double &ay, const double &az, const double &dt) {
    double sig_27 = xe;
    double sig_26 = ze;
    double sig_25 = ye;
    double sq_sig_27 = sig_27 * sig_27,
            sq_sig_26 = sig_26 * sig_26,
            sq_sig_25 = sig_25 * sig_25,
            sq_y_sl_CSPr = y_sl_CSPr * y_sl_CSPr,
            sq_z_sl_CSPr = z_sl_CSPr * z_sl_CSPr;
    double sig_24 = sq_sig_27 - sq_z_sl_CSPr - sq_y_sl_CSPr + sq_sig_25 + sq_sig_26;
    double sig_23 = Vz + 2 * az * dt;
    double sig_22 = Vy + 2 * ay * dt;
    double sig_21 = Vx + 2 * ax * dt;
    double sig_20 = sq_sig_27 + sq_sig_25 + sq_sig_26;
    double sig_19 = sq_sig_27 - sq_z_sl_CSPr + sq_sig_25 + sq_sig_26;
    double sig_18 = sqrt(sq_sig_27 + sq_sig_25);
    double sig_17 = sqrt(sq_y_sl_CSPr / sig_24 + 1);
    double sig_16 = 2 * sig_21 * sig_27 + 2 * sig_22 * sig_25 + 2 * sig_23 * sig_26;
    double sig_15 = sq_sig_25 / sq_sig_27 + 1;
    double sig_14 = sqrt(sig_19 / sig_24) * sqrt(sig_20);
    double sig_13 = z_sl_CSPr * sig_27 * sig_26 + sig_18 * sig_27 * sqrt(sig_19)
                    + (y_sl_CSPr * sq_sig_25 * sig_25) / sqrt(sig_24)
                    + (y_sl_CSPr * sq_sig_27 * sig_25) / sqrt(sig_24)
                    + (y_sl_CSPr * sig_25 * sq_sig_26) / sqrt(sig_24);
    double sig_12 = z_sl_CSPr * sig_18 * sig_17 * sig_16 * sig_24;
    double sig_11 = 2 * sig_21 * sig_27 + 2 * sig_22 * sig_25;
    double sig_10 = sig_15 * sq_sig_27 * sig_19;
    double sig_9 = sig_15 * sig_20 * sq_sig_27 * sig_19;
    double sig_8 = (sig_18 * sig_25 * sqrt(sig_19)) / sqrt(sig_20)
                   - (y_sl_CSPr * sqrt(sig_20) * sig_27) / sqrt(sig_24)
                   + (z_sl_CSPr * sig_25 * sig_26) / sqrt(sig_20);
    double sig_7 = (sig_26 * sqrt(sig_19)) / sig_14
                   - (z_sl_CSPr * sig_18 * sig_17 * sig_24) / (sqrt(sig_20) * sig_19);
    double sq_sig_13 = sig_13 * sig_13;
    double sig_6 = sig_16 * sq_sig_13 * sig_24;
    double r_sig_20 = pow(sig_20, 1.5);
    double sig_5 = 2 * r_sig_20;
    double sig_4 = 2 * pow(sig_24, 1.5);
    double sig_3 = (2 * sig_21 * sq_sig_25) / pow(sig_27, 3) - (2 * sig_22 * sig_25) / sq_sig_27;
    double sq_sig_19 = sig_19 * sig_19;
    double sig_2 = (sig_23 * sqrt(sig_19)) / sig_14
                   - ((sig_16 / sig_24 - (sig_16 * sig_19) / sig_24 * sig_24) * sig_26 * sqrt(sig_19)) /
                     (2 * pow(sig_19 / sig_24, 1.5) * sqrt(sig_20))
                   + (sig_16 * sig_26) / (2 * sqrt(sig_19 / sig_24) * sqrt(sig_20) * sqrt(sig_19))
                   - (sig_16 * sig_26 * sqrt(sig_19)) / (2 * sqrt(sig_19 / sig_24) * r_sig_20)
                   - (z_sl_CSPr * sig_18 * sig_17 * sig_16) / (sqrt(sig_20) * sig_19)
                   + sig_12 / (sqrt(sig_20) * sq_sig_19) + sig_12 / (2 * r_sig_20 * sig_19)
                   - (z_sl_CSPr * sig_11 * sig_17 * sig_24) / (2 * sig_18 * sqrt(sig_20) * sig_19)
                   + (sq_y_sl_CSPr * z_sl_CSPr * sig_18 * sig_16) / (2 * sig_17 * sqrt(sig_20) * sig_19 * sig_24);
    double sq_sig_8 = sig_8 * sig_8,
            sq_sig_7 = sig_7 * sig_7;
    double sig_1 = sq_sig_7 + (sq_sig_8 * sig_24) / sig_10
                   + (sq_sig_13 * sig_24) / sig_9;
    double sq_sig_15 = sig_15 * sig_15,
            sq3_sig_27 = sig_27 * sq_sig_27;
    double Vel = (sig_2 / sqrt(sig_1) - (sig_7 * (2 * sig_7 * sig_2 + (sig_16 * sq_sig_8) / sig_10
                                                  - (2 * sig_21 * sq_sig_8 * sig_24) / (sig_15 * sq3_sig_27 * sig_19)
                                                  - (sig_16 * sq_sig_8 * sig_24) / (sig_15 * sq_sig_27 * sq_sig_19)
                                                  +
                                                  (2 * sig_8 * sig_24 * ((sig_18 * sig_22 * sqrt(sig_19)) / sqrt(sig_20)
                                                                         - (y_sl_CSPr * sig_21 * sqrt(sig_20)) /
                                                                           sqrt(sig_24)
                                                                         + (z_sl_CSPr * sig_23 * sig_25) / sqrt(sig_20)
                                                                         + (z_sl_CSPr * sig_22 * sig_26) / sqrt(sig_20)
                                                                         + (sig_18 * sig_16 * sig_25) /
                                                                           (2 * sqrt(sig_20) * sqrt(sig_19))
                                                                         - (sig_18 * sig_16 * sig_25 * sqrt(sig_19)) /
                                                                           sig_5
                                                                         - (y_sl_CSPr * sig_16 * sig_27) /
                                                                           (2 * sqrt(sig_20) * sqrt(sig_24))
                                                                         +
                                                                         (y_sl_CSPr * sig_16 * sqrt(sig_20) * sig_27) /
                                                                         sig_4
                                                                         + (sig_11 * sig_25 * sqrt(sig_19)) /
                                                                           (2 * sig_18 * sqrt(sig_20))
                                                                         - (z_sl_CSPr * sig_16 * sig_25 * sig_26) /
                                                                           sig_5)) / sig_10
                                                  + (sig_16 * sq_sig_13) / sig_9 +
                                                  (sig_3 * sq_sig_8 * sig_24) / (sq_sig_15 * sq_sig_27 * sig_19)
                                                  + (sig_3 * sq_sig_13 * sig_24) /
                                                    (sq_sig_15 * sig_20 * sq_sig_27 * sig_19)
                                                  + (2 * sig_13 * sig_24 *
                                                     (z_sl_CSPr * sig_23 * sig_27 + z_sl_CSPr * sig_21 * sig_26 +
                                                      sig_18 * sig_21 * sqrt(sig_19)
                                                      + (sig_18 * sig_16 * sig_27) / (2 * sqrt(sig_19))
                                                      + (sig_11 * sig_27 * sqrt(sig_19)) / (2 * sig_18)
                                                      + (y_sl_CSPr * sig_22 * sq_sig_27) / sqrt(sig_24)
                                                      + (3 * y_sl_CSPr * sig_22 * sq_sig_25) / sqrt(sig_24)
                                                      + (y_sl_CSPr * sig_22 * sq_sig_26) / sqrt(sig_24)
                                                      - (y_sl_CSPr * sig_16 * sig_25 * sq_sig_25) / sig_4
                                                      + (2 * y_sl_CSPr * sig_21 * sig_27 * sig_25) / sqrt(sig_24)
                                                      + (2 * y_sl_CSPr * sig_23 * sig_25 * sig_26) / sqrt(sig_24)
                                                      - (y_sl_CSPr * sig_16 * sq_sig_27 * sig_25) / sig_4
                                                      - (y_sl_CSPr * sig_16 * sig_25 * sq_sig_26) / sig_4)) / sig_9
                                                  - (2 * sig_21 * sq_sig_13 * sig_24) /
                                                    (sig_15 * sig_20 * sq3_sig_27 * sig_19)
                                                  - sig_6 / (sig_15 * sig_20 * sq_sig_27 * sq_sig_19)
                                                  - sig_6 / (sig_15 * sig_20 * sig_20 * sq_sig_27 * sig_19))) /
                                        (2 * pow(sig_1, 1.5))) / sqrt(1 - sq_sig_7 / sig_1);
    return Vel;
}

void extrapolationAzEl_V_binocular(const Trajectory &trajectory, const DateTime &time_extra,
                                   double &Az, double &El,
                                   double &VAz, double &VEl) {
    double x_0, y_0, z_0, Vx, Vy, Vz, ax, ay, az;
    double dt = std::chrono::duration<double>(time_extra - trajectory.time0).count();
    transformation_CSSam_to_CSPr(trajectory.x0_hat, trajectory.Vx_hat, trajectory.ax_hat,
                                 trajectory.y0_hat, trajectory.Vy_hat, trajectory.ay_hat,
                                 trajectory.z0_hat, trajectory.Vz_hat, trajectory.az_hat,
                                 x_0, Vx, ax,
                                 y_0, Vy, ay,
                                 z_0, Vz, az);
    double dt2 = dt * dt;
    double xe = ax * dt2 + Vx * dt + x_0,
            ye = ay * dt2 + Vy * dt + y_0,
            ze = az * dt2 + Vz * dt + z_0;
    Az = -Az_CSPr(xe, ye, ze);
    El = El_CSPr(xe, ye, ze);
    VAz = -Vaz_CSPr(xe, ye, ze,
                    Vx, Vy, Vz,
                    ax, ay, az, dt);
    VEl = Vel_CSPr(xe, ye, ze,
                   Vx, Vy, Vz,
                   ax, ay, az, dt);
}

void extrapolationVazVel_4_binocular(const Trajectory &trajectory, const DateTime &time_extra,
                                     double &Vaz, double &Vel) {
    double x_0, y_0, z_0, Vx, Vy, Vz, ax, ay, az;
    double dt = std::chrono::duration<double>(time_extra - trajectory.time0).count();
    transformation_CSSam_to_CSPr(trajectory.x0_hat, trajectory.Vx_hat, trajectory.ax_hat,
                                 trajectory.y0_hat, trajectory.Vy_hat, trajectory.ay_hat,
                                 trajectory.z0_hat, trajectory.Vz_hat, trajectory.az_hat,
                                 x_0, Vx, ax,
                                 y_0, Vy, ay,
                                 z_0, Vz, az);

    double exp2_t = dt * dt;
    double sig_30 = ay * exp2_t + Vy * dt + y_0;
    double sig_29 = ax * exp2_t + Vx * dt + x_0;
    double sig_28 = az * exp2_t + Vz * dt + z_0;

    double exp2_sig_30 = sig_30 * sig_30;
    double exp2_sig_29 = sig_29 * sig_29,
            exp3_sig_29 = sig_29 * exp2_sig_29;
    double exp2_sig_28 = sig_28 * sig_28;
    double exp2_y_sl_CSPr = y_sl_CSPr * y_sl_CSPr;
    double exp2_z_sl_CSPr = z_sl_CSPr * z_sl_CSPr;

    double sig_27 = sqrt(exp2_sig_29 + exp2_sig_30);
    double sig_26 = exp2_sig_29 - exp2_z_sl_CSPr + exp2_sig_30 + exp2_sig_28;
    double sqrt_sig_26 = sqrt(sig_26),
            exp2_sig_26 = sig_26 * sig_26;

    double sig_25 = Vy + 2 * ay * dt;
    double sig_24 = Vz + 2 * az * dt;
    double sig_23 = Vx + 2 * ax * dt;
    double sig_22 = exp2_sig_29 + exp2_sig_30 + exp2_sig_28;
    double sig_21 = exp2_sig_29 - exp2_z_sl_CSPr - exp2_y_sl_CSPr + exp2_sig_30 + exp2_sig_28;
    double sqrt_sig_21 = sqrt(sig_21),
            exp2_sig_21 = sig_21 * sig_21;
    double sqrt_sig_22 = sqrt(sig_22);

    double sig_20 = exp2_sig_30 / exp2_sig_29 + 1;
    double sig_19 = z_sl_CSPr * sig_27 - sig_28 * sqrt_sig_26;
    double exp2_sig_19 = sig_19 * sig_19;

    double sig_18 = 2 * sig_23 * sig_29 + 2 * sig_25 * sig_30 + 2 * sig_24 * sig_28;
    double sig_17 = 2 * sig_23 * sig_29 + 2 * sig_25 * sig_30;
    double sig_16 = 2 * sqrt_sig_22 * sqrt_sig_21;
    double sig_15 = 2 * sqrt_sig_22 * sqrt_sig_26;
    double sig_14 = 2 * pow(sig_21, 1.5);
    double sig_13 = 2 * pow(sig_22, 1.5);
    double sig_12 = sig_20 * exp2_sig_29 * sig_26;
    double sig_11 = (sig_27 * sig_29 * sqrt_sig_26) / sqrt_sig_22
                    + (y_sl_CSPr * sqrt_sig_22 * sig_30) / sqrt_sig_21 + (z_sl_CSPr * sig_29 * sig_28) / sqrt_sig_22;
    double sig_10 = (sig_27 * sig_30 * sqrt_sig_26) / sqrt_sig_22
                    - (y_sl_CSPr * sqrt_sig_22 * sig_29) / sqrt_sig_21 + (z_sl_CSPr * sig_30 * sig_28) / sqrt_sig_22;
    double exp2_sig_11 = sig_11 * sig_11;
    double exp2_sig_10 = sig_10 * sig_10;

    double sig_9 = exp2_sig_19 * sig_18 * sig_21;
    double sig_8 = sig_20 * exp3_sig_29 * sig_26;
    double sig_7 = sig_20 * exp2_sig_29 * exp2_sig_26;
    double sig_6 = sig_20 * sig_20 * exp2_sig_29 * sig_26;
    double sig_5 = (2 * sig_23 * exp2_sig_30) / exp3_sig_29 - (2 * sig_25 * sig_30) / exp2_sig_29;
    double sig_4 = sig_24 * sqrt_sig_26 - (z_sl_CSPr * sig_17) / (2 * sig_27) + (sig_18 * sig_28) / (2 * sqrt_sig_26);
    double sig_3 = (sig_27 * sig_23 * sqrt_sig_26) / sqrt_sig_22 + (y_sl_CSPr * sig_25 * sqrt_sig_22) / sqrt_sig_21
                   + (z_sl_CSPr * sig_24 * sig_29) / sqrt_sig_22 + (z_sl_CSPr * sig_23 * sig_28) / sqrt_sig_22
                   + (sig_27 * sig_18 * sig_29) / sig_15 - (sig_27 * sig_18 * sig_29 * sqrt_sig_26) / sig_13
                   + (y_sl_CSPr * sig_18 * sig_30) / sig_16 - (y_sl_CSPr * sig_18 * sqrt_sig_22 * sig_30) / sig_14
                   + (sig_17 * sig_29 * sqrt_sig_26) / (2 * sig_27 * sqrt_sig_22) -
                   (z_sl_CSPr * sig_18 * sig_29 * sig_28) / sig_13;
    double sig_2 = (sig_27 * sig_25 * sqrt_sig_26) / sqrt_sig_22 - (y_sl_CSPr * sig_23 * sqrt_sig_22) / sqrt_sig_21
                   + (z_sl_CSPr * sig_24 * sig_30) / sqrt_sig_22 + (z_sl_CSPr * sig_25 * sig_28) / sqrt_sig_22
                   + (sig_27 * sig_18 * sig_30) / sig_15 - (sig_27 * sig_18 * sig_30 * sqrt_sig_26) / sig_13
                   - (y_sl_CSPr * sig_18 * sig_29) / sig_16 + (y_sl_CSPr * sig_18 * sqrt_sig_22 * sig_29) / sig_14
                   + (sig_17 * sig_30 * sqrt_sig_26) / (2 * sig_27 * sqrt_sig_22) -
                   (z_sl_CSPr * sig_18 * sig_30 * sig_28) / sig_13;
    double sig_1 = (exp2_sig_19 * sig_21) / (sig_22 * sig_26) + (exp2_sig_11 * sig_21) / sig_12 +
                   (exp2_sig_10 * sig_21) / sig_12;
    double sqrt_sig_1 = sqrt(sig_1);

    Vaz = - (sig_2 / sig_11 - (sig_10 * sig_3) / exp2_sig_11) / (exp2_sig_10 / exp2_sig_11 + 1);
    Vel = (sig_4 / ((sqrt_sig_26 / sqrt_sig_21) * sqrt_sig_1 * sqrt_sig_22)
           + (sig_19 * (sig_18 / sig_21 - (sig_18 * sig_26) / exp2_sig_21)) / (2 * pow(sig_26 / sig_21, 1.5) *
                                                                               sqrt_sig_1 * sqrt_sig_22)
           + (sig_19 * sig_18) / (2 * (sqrt_sig_26 / sqrt_sig_21) * sqrt_sig_1 * pow(sig_22, 1.5))
           + (sig_19 *
              ((exp2_sig_19 * sig_18) / (sig_22 * sig_26) - (2 * sig_19 * sig_4 * sig_21) / (sig_22 * sig_26)
               - sig_9 / (sig_22 * exp2_sig_26) - sig_9 / (sig_22 * sig_22 * sig_26) + (sig_18 * exp2_sig_11) / sig_12
               + (sig_18 * exp2_sig_10) / sig_12 - (2 * sig_23 * exp2_sig_11 * sig_21) / sig_8
               - (2 * sig_23 * exp2_sig_10 * sig_21) / sig_8 - (sig_18 * exp2_sig_11 * sig_21) / sig_7
               - (sig_18 * exp2_sig_10 * sig_21) / sig_7 + (2 * sig_11 * sig_21 * sig_3) / sig_12 +
               (2 * sig_10 * sig_21 * sig_2) / sig_12
               + (sig_5 * exp2_sig_11 * sig_21) / sig_6 + (sig_5 * exp2_sig_10 * sig_21) / sig_6))
             / (2 * (sqrt_sig_26 / sqrt_sig_21) * pow(sig_1, 1.5) * sqrt_sig_22)) /
          sqrt(1 - (exp2_sig_19 * sig_21) / (sig_1 * sig_22 * sig_26));
}

void extrapolation_Az_El_Vaz_Vel_binocular(const Trajectory &trajectory, const DateTime &time_extra,
                                           double &Az, double &El,
                                           double &VAz, double &VEl) {
    double x_0, y_0, z_0, Vx, Vy, Vz, ax, ay, az;
    double dt = std::chrono::duration<double>(time_extra - trajectory.time0).count();
    transformation_CSSam_to_CSPr(trajectory.x0_hat, trajectory.Vx_hat, trajectory.ax_hat,
                                 trajectory.y0_hat, trajectory.Vy_hat, trajectory.ay_hat,
                                 trajectory.z0_hat, trajectory.Vz_hat, trajectory.az_hat,
                                 x_0, Vx, ax,
                                 y_0, Vy, ay,
                                 z_0, Vz, az);
    double ex2_t = dt * dt;
    double sig_7 = ax * ex2_t + Vx * dt + x_0 - x_sl_CSPr;
    double sig_6 = ay * ex2_t + Vy * dt + y_0 - y_sl_CSPr;
    double sig_5 = az * ex2_t + Vz * dt + z_0 - z_sl_CSPr;
    double ex2_sig_7 = sig_7 * sig_7;
    double ex2_sig_6 = sig_6 * sig_6;
    double ex2_sig_5 = sig_5 * sig_5;
    double sig_4 = ex2_sig_6 + ex2_sig_5 + ex2_sig_7;
    double sig_3 = Vx + 2 * ax * dt;
    double sig_2 = Vy + 2 * ay * dt;
    double sig_1 = Vz + 2 * az * dt;

    Az = atan(sig_6 / sig_7);
    El = asin(sig_5 / sqrt(sig_4));
    VAz = (sig_2 / sig_7 - (sig_3 * sig_6) / ex2_sig_7) / (ex2_sig_6 / ex2_sig_7 + 1);
    VEl = (sig_1 / sqrt(sig_4) -
           ((2 * sig_2 * sig_6 + 2 * sig_1 * sig_5 + 2 * sig_3 * sig_7) * sig_5) / (2 * pow(sig_4, 1.5))) /
          sqrt(1 - ex2_sig_5 / sig_4);
}