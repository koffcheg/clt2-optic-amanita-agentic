#ifndef DATAPRO2TYPES_H
#define DATAPRO2TYPES_H
#include <opencv2/opencv.hpp>
#include "datapro1/src/datarpoTypes.h"

using DateTime = std::chrono::time_point<std::chrono::system_clock>;

struct Measurement {
    int id_obj;
    double x, y;
    int iframe;
    double Xp, Yp, Zp;
    DateTime time;
    double Az, El;
    double dispersionX=0, dispersionY=0, dispersionZ=0;
};

struct PTPoint {
    Measurement measurement;
    int passed_frames = 1;
};

struct TStrobe{
    unsigned long id_trajectory;
    double x_0, y_0, z_0;
    double x_min, y_min, z_min;
    double x_max, y_max, z_max;
};

struct Trajectory {
    std::deque<Measurement> measurements;
    unsigned long id;
    int cam_index;
    DateTime time0;
    double x0_hat = 0;
    double y0_hat = 0;
    double z0_hat = 0;
    double Vx_hat = 0;
    double Vy_hat = 0;
    double Vz_hat = 0;
    double ax_hat = 0;
    double ay_hat = 0;
    double az_hat = 0;
    int unconfermed_frames_count = 0;
    bool updated;
    double speed = 0;
    double acceleration = 0;
    double std_dev_x = 0;
    double std_dev_y = 0;
    double std_dev_z = 0;
    double std_dev_xyz = 0;
    TStrobe strobe_trj{};
};



#endif //DATAPRO2TYPES_H
