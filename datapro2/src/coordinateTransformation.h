//
// Created by user on 05.08.24.
//

#ifndef CLT_OPTIC_COORDINATETRANSFORMATION_H
#define CLT_OPTIC_COORDINATETRANSFORMATION_H
#include "datapro2Types.h"

double toRadians(double degrees);

void xy2XYZ(const double &x_frame, const double &y_frame,
            const double &focalLength_x, const double &focalLength_y,
            const double &u0, const double &v0, const double &distance,
            const double &x_cam0, const double &y_cam0, const double &z_cam0,
            const double &Az, const double &El,
            double &x_3d, double &y_3d, double &z_3d);

void xy2XYZ(const TDataFrame &data_frame, const TDataCam &data_cam, const double &x_frame, const double &y_frame,
            const double &f_x, const double &f_y, const double &c_x, const double &c_y,
            double &x_3d, double &y_3d, double &z_3d);

void XYZ2xy(const double &x_3d, const double &y_3d, const double &z_3d,
            const double &focalLength_x, const double &focalLength_y,
            const double &u0, const double &v0,
            const double &x_cam0, const double &y_cam0, const double &z_cam0,
            const double &Az, const double &El,
            double &x_frame, double &y_frame);

void extrapolationAzEl_V_a(const Trajectory &trajectory, const DateTime &time_extra,
                           double &Az, double &El,
                           double &VAz, double &VEl,
                           double &aAz, double &aEl);

void  extrapolation_coor(const double &x0_hat, const double &Vx_hat, const double &ax_hat, const double &dt,
                         double& Xp, double& Vx, double& ax);

void extrapolationAzEl_4_binocular(const Trajectory &trajectory, const DateTime &time_extra,
                                   double &Az, double &El);

void extrapolationAzEl_V_4_binocular(const Trajectory &trajectory, const DateTime &time_extra,
                                   double &Az, double &El,
                                   double &VAz, double &VEl);

void extrapolationAzEl_V_binocular(const Trajectory &trajectory, const DateTime &time_extra,
                                   double &Az, double &El,
                                   double &VAz, double &VEl);

void extrapolationVazVel_4_binocular(const Trajectory &trajectory, const DateTime &time_extra,
                                     double &Vaz, double &Vel);

void extrapolation_Az_El_Vaz_Vel_binocular(const Trajectory &trajectory, const DateTime &time_extra,
                                           double &Az, double &El,
                                           double &VAz, double &VEl);
#endif //CLT_OPTIC_COORDINATETRANSFORMATION_H
