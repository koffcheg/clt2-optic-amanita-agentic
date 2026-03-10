#ifndef FRAGMENTATION_H
#define FRAGMENTATION_H
#include "datarpoTypes.h"
#include <opencv2/opencv.hpp>
#include <vector>
#include "dp1_config.h"

std::vector<cv::Mat> splitImage2Fragments(const cv::Mat& image, const int &border_x, const int &border_y, const int& numFragX, const int& numFragY);
void calcNumberFrag(unsigned int tiles_factor, const int& im_width, const int& im_height, int& numFragX, int& numFragY);
void calcBorderSize(const ns_datapro1::prg_config::cfg_one_filter&corr, const ns_datapro1::prg_config::cfg_one_filter&matched,
	const ns_datapro1::prg_config::cfg_one_filter&blur, const int& def_border,
	int &border_x, int &border_y);
cv::Mat splitImage(const cv::Mat& image, const int &border_x, const int &border_y, const int& numFragX, const int& numFragY, const int &width, const int &height,
	const int &width_last_column, const int &height_last_row, const int &i, const int &j);
void calcSizePart(const int& sizeYin, const int& sizeXin, const int& numFragY, const int& numFragX,
	int& sizePartY, int& sizePartX, int& sizePartYend, int& sizePartXend);
#endif
