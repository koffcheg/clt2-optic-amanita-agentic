//
// Created by u on 24.07.24.
// data marshaling dp1 --> dp2
//

#ifndef CLT_OPTIC_DP1_RPC_DATA_MRSH_H
#define CLT_OPTIC_DP1_RPC_DATA_MRSH_H

class CMemStore;
struct TDataRes;
namespace cv{class Mat;}
struct TDataCalibrationCamera;
struct TDataCalibrationFrame;

void serialize_dp1_res(CMemStore &, const TDataRes&);
bool deserialize_dp1_res(CMemStore &, TDataRes&);

void serialize_Mat(CMemStore &ms, const cv::Mat &);
void deserialize_Mat(CMemStore &ms, cv::Mat &img);

void serialize_camera_calibration_data(CMemStore &ms, const TDataCalibrationCamera &);
void deserialize_camera_calibration_data(CMemStore &ms, TDataCalibrationCamera &);

void serialize_frame_calibration_data(CMemStore &ms, const TDataCalibrationFrame &);
void deserialize_frame_calibration_data(CMemStore &ms, TDataCalibrationFrame &);

#endif //CLT_OPTIC_DP1_RPC_DATA_MRSH_H
