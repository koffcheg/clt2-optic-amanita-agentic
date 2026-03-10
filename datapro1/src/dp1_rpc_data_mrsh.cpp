#include "dp1_rpc_data_mrsh.h"
#include "mem_store.h"
#include "datarpoTypes.h"
#include <log4cxx/logger.h>

static auto logger = log4cxx::Logger::getLogger("dp1-data-marsh");

void serialize_Mat(CMemStore &ms, const cv::Mat &img){
    ms.write_native<int>(img.cols);
    ms.write_native<int>(img.rows);
    ms.write_native<int>(static_cast<int>(img.elemSize()));
    ms.write_native<int>(img.type());

	int rows_to_proc = img.rows;
	int cols_to_proc = img.cols;
	if(img.isContinuous()){
		cols_to_proc *= rows_to_proc;
		rows_to_proc = 1;
	}
	for(int save_row_index = 0; save_row_index < rows_to_proc; ++save_row_index){
		auto *ptr = img.ptr(save_row_index);
		unsigned int bytes_to_write = cols_to_proc*img.elemSize()*img.channels();
		ms.write(ptr, bytes_to_write);
	}
}

void deserialize_Mat(CMemStore &ms, cv::Mat &img) {
    int cols, rows, elemSizeInBytes, elemType;
    ms.read_native(cols);
    ms.read_native(rows);
    ms.read_native(elemSizeInBytes);
    ms.read_native(elemType);

	cv::Mat tmp_mat(rows, cols, elemType, ms.getCurrPtr()); // OR vec.data() instead of ptr
	ms.read(nullptr, cols*rows*elemSizeInBytes, false);
	tmp_mat.copyTo(img);
}

void serialize_camera_calibration_data(CMemStore &ms, const TDataCalibrationCamera &data){
	serialize_Mat(ms, data.cameraMatrix);
	serialize_Mat(ms, data.distCoeffs);

	ms.write_native(data.avg_reprojection_error);
	ms.write_native(data.square_size);
	ms.write_native(data.board_width);
	ms.write_native(data.board_height);
}

void deserialize_camera_calibration_data(CMemStore &ms, TDataCalibrationCamera &data){
	deserialize_Mat(ms, data.cameraMatrix);
	deserialize_Mat(ms, data.distCoeffs);

	ms.read_native(data.avg_reprojection_error);
	ms.read_native(data.square_size);
	ms.read_native(data.board_width);
	ms.read_native(data.board_height);
}

void serialize_frame_calibration_data(CMemStore &ms, const TDataCalibrationFrame &data){
	serialize_Mat(ms, data.R_matrix);
	serialize_Mat(ms, data.t_vec);
	serialize_Mat(ms, data.r_vec);
	serialize_Mat(ms, data.descriptors_scene);

	ms.write_native<unsigned int>(data.chessboard_corners.size());
	ms.write(&data.chessboard_corners[0], data.chessboard_corners.size()*sizeof(cv::Point2f));

	ms.write_native<unsigned int>(data.keypoints_scene.size());
	ms.write(&data.keypoints_scene[0], data.keypoints_scene.size()*sizeof(cv::KeyPoint));
}

void deserialize_frame_calibration_data(CMemStore &ms, TDataCalibrationFrame &data){
	deserialize_Mat(ms, data.R_matrix);
	deserialize_Mat(ms, data.t_vec);
	deserialize_Mat(ms, data.r_vec);
	deserialize_Mat(ms, data.descriptors_scene);

	unsigned int vec_size;

	ms.read_native(vec_size);
	data.chessboard_corners.resize(vec_size);
	ms.read(&data.chessboard_corners[0], vec_size*sizeof(cv::Point2f));

	ms.read_native(vec_size);
	data.keypoints_scene.resize(vec_size);
	ms.read(&data.keypoints_scene[0], vec_size*sizeof(cv::KeyPoint));
}

void serialize_dp1_res(CMemStore &ms, const TDataRes &data) {

	//TDataCam members
	ms.write_native<uint32_t>(sizeof(data.data_cam));
	ms.write_native(data.data_cam);

	//TDataFrame members
	ms.write_native<uint32_t>(sizeof(data.data_frame));
	ms.write_native(data.data_frame);

    //std::vector<TOptionsMeasurement> meas members
	ms.write_native<uint32_t>(sizeof(TOptionsMeasurement));
	ms.write_native<uint32_t>(data.meas.size());
	for (const auto &meas_el: data.meas)
		ms.write_native(meas_el);

	serialize_frame_calibration_data(ms, data.calib_frame);
}

bool deserialize_dp1_res(CMemStore &ms, TDataRes &data) {
	constexpr std::size_t min_msg_size =
			4 + sizeof(data.data_cam) +
			4 + sizeof(data.data_frame) +
			8;
	if (min_msg_size > ms.getCurrFreeLimit()) {
		LOG4CXX_ERROR(logger, "message size is not enough to deserialize TDataRes: need " << min_msg_size << ", rc: "
																						  << ms.getCurrFreeLimit());
		return false;
	}
	uint32_t struct_size;
	ms.read_native(struct_size);
	if (struct_size != sizeof(data.data_cam)) {
		LOG4CXX_ERROR(logger, ".data_cam size mismatch, rc: " << struct_size << ", real: " << sizeof(data.data_cam));
		return false;
	}
	ms.read_native(data.data_cam);

	ms.read_native(struct_size);
	if (struct_size != sizeof(data.data_frame)) {
		LOG4CXX_ERROR(logger, ".data_frame size mismatch, rc: " << struct_size << ", real: " << sizeof(data.data_cam));
		return false;
	}
	ms.read_native(data.data_frame);

	ms.read_native(struct_size);
	if (struct_size != sizeof(TOptionsMeasurement)) {
		LOG4CXX_ERROR(logger,
					  "TOptionsMeasurement size mismatch, rc: " << struct_size << ", real: " << sizeof(data.data_cam));
		return false;
	}
    uint32_t num_rec;
	ms.read_native(num_rec);
	uint32_t need_size = num_rec * sizeof(TOptionsMeasurement);
	if (need_size > ms.getCurrFreeLimit()) {
		LOG4CXX_ERROR(logger, "wrong data size left, need: " << need_size << ", real: " << ms.getCurrFreeLimit());
		return false;
	}
	data.meas.clear();
	TOptionsMeasurement rd_meas;
	for (uint32_t i = 0; i < num_rec; ++i) {
		ms.read_native(rd_meas);
		data.meas.push_back(rd_meas);
	}

	deserialize_frame_calibration_data(ms, data.calib_frame);

	return true;
}
