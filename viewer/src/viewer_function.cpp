//
// Created by user on 11.09.24.
//
#include "viewer_function.h"
#include <jansson.h>

bool get_arg(const cv::CommandLineParser& parser, int arg_index, std::string &res, const char *err_msg)
{
    res = parser.get<cv::String>(arg_index);
    if (!std::filesystem::exists(res)) {
        std::cerr << "ERROR: path for " << err_msg << " is absent (or wrong)";
        return false;
    }
    return true;
}

bool get_arg(const cv::CommandLineParser& parser, int arg_index, int& res, const char *err_msg)
{
    res = parser.get<int>(arg_index);
    if (arg_index==3){
        if (!(res==0||res==1)) {
            std::cerr << "ERROR: parameter " << err_msg << " is absent (or wrong)";
            return false;
        }
    } else if(arg_index==4){
        if ((res<5||res>30)) {
            std::cerr << "ERROR: parameter " << err_msg << " is absent (or wrong)";
            return false;
        }
    }
    return true;
}

std::string compose_file_name(const std::string &path, const std::string &filename, const std::string &ext){
    fs::path full_path{path};
    full_path /= filename;
    full_path += ext;
    return full_path.string();
}

std::vector<std::string> creating_list_files_folder(const std::string& path, const std::string& ext){
    std::vector<std::string> ls_file;
    for (auto &p : fs::recursive_directory_iterator(path)){
        if (p.path().extension() == ext){
            ls_file.push_back(p.path().stem().string());
        }
//            std::cout << p.path().stem().string() << '\n';
    }
    std::sort(ls_file.begin(), ls_file.end());
    return ls_file;
}

template<typename T>
void read_from_binary(std::ifstream &file_bin, T &value) {
    file_bin.read(reinterpret_cast<char *>(&value), sizeof(value));
}

void extrapolation_trajectory(const Trajectory& trajectory, const DateTime &time_extra,
                              double& Xe, double& Ye, double& Ze){
    double dt = std::chrono::duration<double>(time_extra - trajectory.time0).count();
    double Vxe=0, axe=0, Vye=0, aye=0, Vze=0, aze=0;
    extrapolation_coor(trajectory.x0_hat, trajectory.Vx_hat, trajectory.ax_hat, dt,
                       Xe, Vxe, axe);
    extrapolation_coor(trajectory.y0_hat, trajectory.Vy_hat, trajectory.ay_hat, dt,
                       Ye, Vye, aye);
    extrapolation_coor(trajectory.z0_hat, trajectory.Vz_hat, trajectory.az_hat, dt,
                       Ze, Vze, aze);
}
//
//rgb(0,2,122)
//rgb(0,78,92)
//rgb(0,154,62)
//rgb(0,230,31)
void read_res_dp2(std::ifstream &file_bin, Trajectory &result) {
    unsigned long size;
    double tmp;
    read_from_binary(file_bin, result.id);
    read_from_binary(file_bin, result.cam_index);
    read_from_binary(file_bin, result.time0);
    read_from_binary(file_bin, result.x0_hat);
    read_from_binary(file_bin, result.y0_hat);
    read_from_binary(file_bin, result.z0_hat);
    read_from_binary(file_bin, result.Vx_hat);
    read_from_binary(file_bin, result.Vy_hat);
    read_from_binary(file_bin, result.Vz_hat);
    read_from_binary(file_bin, result.ax_hat);
    read_from_binary(file_bin, result.ay_hat);
    read_from_binary(file_bin, result.az_hat);
    read_from_binary(file_bin, result.unconfermed_frames_count);
    read_from_binary(file_bin, result.updated);
    read_from_binary(file_bin, result.speed);
    read_from_binary(file_bin, result.acceleration);
    read_from_binary(file_bin, result.std_dev_x);
    read_from_binary(file_bin, result.std_dev_y);
    read_from_binary(file_bin, result.std_dev_z);
    read_from_binary(file_bin, result.std_dev_xyz);

    read_from_binary(file_bin, result.strobe_trj);

    read_from_binary(file_bin, tmp);
    read_from_binary(file_bin, tmp);
    read_from_binary(file_bin, tmp);
    read_from_binary(file_bin, tmp);
    read_from_binary(file_bin, tmp);
    read_from_binary(file_bin, tmp);

    read_from_binary(file_bin, size);
    result.measurements.resize(size);
    for (auto &m : result.measurements) {
        read_from_binary(file_bin, m.id_obj);
        read_from_binary(file_bin, m.x);
        read_from_binary(file_bin, m.y);
        read_from_binary(file_bin, m.iframe);
        read_from_binary(file_bin, m.Xp);
        read_from_binary(file_bin, m.Yp);
        read_from_binary(file_bin, m.Zp);
        read_from_binary(file_bin, m.time);
        read_from_binary(file_bin, m.Az);
        read_from_binary(file_bin, m.El);
    }
}

void read_trajectories_from_binary(const std::string &file_path, TDataPro2& data_dp2, double& k_sigma) {
    std::ifstream file_bin(file_path, std::ios::in | std::ios::binary);
    if (!file_bin) {
        throw std::runtime_error("Error opening file");
    }
    unsigned long size;
    read_from_binary(file_bin, data_dp2.cam_index);
    read_from_binary(file_bin, data_dp2.Xcam);
    read_from_binary(file_bin, data_dp2.Ycam);
    read_from_binary(file_bin, data_dp2.Zcam);
    read_from_binary(file_bin, data_dp2.focal_length_x);
    read_from_binary(file_bin, data_dp2.focal_length_y);
    read_from_binary(file_bin, data_dp2.optical_centers_x);
    read_from_binary(file_bin, data_dp2.optical_centers_y);
    read_from_binary(file_bin, data_dp2.frame_index);
    read_from_binary(file_bin, k_sigma);

    read_from_binary(file_bin, size);
    data_dp2.trajectories.resize(size);
    for (auto &traj : data_dp2.trajectories) {
        read_res_dp2(file_bin, traj);
    }
    file_bin.close();
}

void calc_frame_dp1(cv::Mat& im, const std::string& path_bin_dp1, cv::Mat& frame, int& check_binocular){
    TDataCalibrationCamera param_cam;
    TDataCalibrationFrame param_frame;
    TDataRes data_bin_dp1 = read_res(path_bin_dp1, check_binocular, param_cam, param_frame);
    std::string txt;
    std::ostringstream strs;
    cv::RotatedRect rRect;
    cv::Point2f vertices[4];
    for (const auto &el: data_bin_dp1.meas) {
        cv::drawMarker(im,cv::Point(el.x_weight,el.y_weight),
                       cv::Scalar(0, 0, 255),
                       cv::MARKER_CROSS,
                       10,1);

        rRect = cv::RotatedRect(cv::Point2f(el.x_rec,el.y_rec),
                                cv::Size2f(el.rec_width,el.rec_height), el.obj_angel);// /180*3.141592
        rRect.points(vertices);
        for (int i = 0; i < 4; i++){
            line(im, vertices[i], vertices[(i+1)%4], cv::Scalar(0,0,255));//, 2
        }

        strs << "id: " << el.id_obj;
        cv::putText(im, strs.str(), cv::Point(el.x_weight,el.y_weight),
                    cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar(0, 0, 255), 1, cv::LINE_8);
        strs.str("");
        strs.clear();
    }
    frame = im.clone();
}

void convert_strobe_to_2D(const TStrobe& strobe, const TDataPro2& data, const Trajectory& traj,
                          cv::Point& point1, cv::Point& point2,cv::Point& point3,cv::Point& point4){
    double x_frame1, y_frame1;
    XYZ2xy(strobe.x_0, strobe.y_min, strobe.z_min,
           data.focal_length_x, data.focal_length_y, data.optical_centers_x, data.optical_centers_y,
           data.Xcam, data.Ycam, data.Zcam, traj.measurements.back().Az, traj.measurements.back().El,
           x_frame1, y_frame1);
    point1 = cv::Point(x_frame1, y_frame1);
    XYZ2xy(strobe.x_0, strobe.y_min, strobe.z_max,
           data.focal_length_x, data.focal_length_y, data.optical_centers_x, data.optical_centers_y,
           data.Xcam, data.Ycam, data.Zcam, traj.measurements.back().Az, traj.measurements.back().El,
           x_frame1, y_frame1);
    point2 = cv::Point(x_frame1, y_frame1);
    XYZ2xy(strobe.x_0, strobe.y_max, strobe.z_max,
           data.focal_length_x, data.focal_length_y, data.optical_centers_x, data.optical_centers_y,
           data.Xcam, data.Ycam, data.Zcam, traj.measurements.back().Az, traj.measurements.back().El,
           x_frame1, y_frame1);
    point3 = cv::Point(x_frame1, y_frame1);
    XYZ2xy(strobe.x_0, strobe.y_max, strobe.z_min,
           data.focal_length_x, data.focal_length_y, data.optical_centers_x, data.optical_centers_y,
           data.Xcam, data.Ycam, data.Zcam, traj.measurements.back().Az, traj.measurements.back().El,
           x_frame1, y_frame1);
    point4 = cv::Point(x_frame1, y_frame1);
}

void calc_frame_dp2(cv::Mat& im, const std::string& path_bin_dp1,
                    const std::vector<std::string>& files_name,
                    const size_t& index_current_frame,
                    cv::Mat& frame){
    TDataPro2 data;
    double k_sigma;
    std::vector<TDataPro2>  vector_data;
    int index;
    std::vector<cv::Scalar> array_color = {cv::Scalar(31, 230, 0),
                                           cv::Scalar(62, 154, 0),
                                           cv::Scalar(92, 78, 0),
                                           cv::Scalar(122, 2, 0)};
//    std::vector<cv::Scalar> array_color = {cv::Scalar(122, 2, 0),
//                                           cv::Scalar(92, 78, 0),
//                                           cv::Scalar(62, 154, 0),
//                                           cv::Scalar(31, 230, 0)};

    for (int i = 0; i < 4; i++) {
        index = (int)index_current_frame - i;
        if(index >= 0){
            std::string path_bin_dp1_current_frame = compose_file_name(path_bin_dp1, files_name[index],
                                                                       ".blob");
            read_trajectories_from_binary(path_bin_dp1_current_frame, data, k_sigma);
            vector_data.push_back(data);
        }
    }
    int r=4;
    std::string txt;
    std::ostringstream strs;
    Measurement extrapolate;
    TStrobe strobe{};
    cv::Point point1,point2,point3,point4;

    cv::Scalar clr = cv::Scalar(122, 2, 0);
    //
    double x_frame0, y_frame0, x_frame1, y_frame1, dispersion_xp, dispersion_yp, dispersion_zp;
    double Xe0, Ye0, Ze0, Xe1, Ye1, Ze1;
    unsigned long id_tr;
    DateTime current_tame_tr_fr;
        for (const auto &traj: vector_data[0].trajectories) {
            id_tr = traj.id;
            current_tame_tr_fr = traj.measurements.back().time;
                for (int j = 0; j < traj.measurements.size() - 1; j++) {
                    cv::line(im, cv::Point(traj.measurements[j].x, traj.measurements[j].y),
                             cv::Point(traj.measurements[j + 1].x, traj.measurements[j + 1].y),
                             cv::Scalar(126, 83, 22),//B G R  rgb(22,83,126)
                                                      3);
                }
                for (const auto &el: traj.measurements) {
                    cv::circle(im, cv::Point(el.x, el.y), r, cv::Scalar(126, 83, 22), -1);
                }
                for (int j = 0; j < traj.measurements.size() - 1; j++) {
                    extrapolation_trajectory(traj, traj.measurements[j].time,
                                             Xe0, Ye0, Ze0);
                    extrapolation_trajectory(traj, traj.measurements[j + 1].time,
                                             Xe1, Ye1, Ze1);
                    XYZ2xy(Xe0, Ye0, Ze0,
                           data.focal_length_x, data.focal_length_y, data.optical_centers_x, data.optical_centers_y,
                           data.Xcam, data.Ycam, data.Zcam, traj.measurements[j].Az, traj.measurements[j].El,
                           x_frame0, y_frame0);
                    XYZ2xy(Xe1, Ye1, Ze1,
                           data.focal_length_x, data.focal_length_y, data.optical_centers_x, data.optical_centers_y,
                           data.Xcam, data.Ycam, data.Zcam, traj.measurements[j + 1].Az, traj.measurements[j + 1].El,
                           x_frame1, y_frame1);
                    cv::line(im, cv::Point(x_frame0, y_frame0),
                             cv::Point(x_frame1, y_frame1),
                             array_color[3],//cv::Scalar(0, 0, 255),//cv::Scalar(122, 2, 0),//B G R  rgb(0,2,122)
                             2);
                }

            for (int i = 0; i<4; i++) {
                for (const auto &traj2: vector_data[i].trajectories) {
                    if (traj2.id==id_tr){
                        convert_strobe_to_2D(traj2.strobe_trj, vector_data[i], traj2,
                                             point1, point2, point3, point4);
                        cv::line(im, point1, point2, array_color[i], 2);//cv::Scalar(0, 0, 255),//cv::Scalar(122, 2, 0),//B G R  rgb(0,2,122)
                        cv::line(im, point2, point3, array_color[i], 2);
                        cv::line(im, point3, point4, array_color[i], 2);
                        cv::line(im, point4, point1, array_color[i], 2);
                    }
                }
            }
                cv::drawMarker(im, cv::Point(traj.measurements.back().x, traj.measurements.back().y),
                               cv::Scalar(126, 83, 22),
                               cv::MARKER_TILTED_CROSS,
                               20, 1);
                strs << "tr: " << traj.id;
                cv::putText(im, strs.str(), cv::Point(traj.measurements.back().x, traj.measurements.back().y),
                            cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar(126, 83, 22), 1, cv::LINE_8);
                strs.str("");
                strs.clear();
        }
    frame = im.clone();
}