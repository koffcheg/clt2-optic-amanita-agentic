#ifndef DATAPROSAVETOFILE_H
#define DATAPROSAVETOFILE_H
#include <vector>

#include "datapro2Types.h"


void save_trajectories_as_blob(const std::string &path, const TDataCam &data_cam, const TDataFrame &data_frame,
                               const double& focal_length_x, const double& focal_length_y,
                               const double& optical_centers_x, const double& optical_centers_y, const double& k_sigma,
                               const std::vector<Trajectory> &trajectories, const bool& enabled_binocular) ;

void save_trajectories_as_plain_text(const std::string &path, const TDataCam &data_cam, const TDataFrame &data_frame,
                                     const std::vector<Trajectory> &trajectories, const bool& enabled_binocular);

void save_trajectories_as_json(const std::string &path, const TDataCam &data_cam, const TDataFrame &data_frame,
                               const double& focal_length_x, const double& focal_length_y,
                               const double& optical_centers_x, const double& optical_centers_y,
                               const std::vector<Trajectory> &trajectories);

#endif //DATAPROSAVETOFILE_H
