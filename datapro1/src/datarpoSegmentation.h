#ifndef SEGMENTATION_H
#define SEGMENTATION_H
#include <opencv2/opencv.hpp>
#include <vector>
#include "datarpoTypes.h"

std::vector<std::vector<cv::Point> > refineSegments(const cv::Mat& mask, const int& weight_element, const int& weight_height, const int& niters,
	const int& type_element, const int& level, const int& level_max, const int& threshold_type,
	const int& contour_retrieval, const int& contour_approximation, const int& shift_x, const int& shift_y);
//std::vector<TOptionsMeasurement> calcMeasurement(const cv::Mat& img, const std::vector<std::vector<cv::Point> >& contours,
//                                                 const int& min_segment_size, const int& max_segment_size,
//                                                 std::vector<TDrawMeasurement>& data_draw);
std::vector<TOptionsMeasurement> calcMeasurement(const cv::Mat& img, const std::vector<std::vector<cv::Point> >& contours,
                                                 const std::vector<TDrawMeasurement>& data_draw);
//std::vector<TDrawMeasurement> calc_contour_param(const std::vector<std::vector<cv::Point>>& contours,
//                                                 const int& min_segment_size, const int& max_segment_size);
void calc_contour_param(const int& min_segment_size, const int& max_segment_size, const double& max_height_to_width_ratio,
                        std::vector<std::vector<cv::Point>>& contours,
                        std::vector<TDrawMeasurement>& data_draw);
#endif