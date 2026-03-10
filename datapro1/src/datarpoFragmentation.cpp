#include "datarpoFragmentation.h"
#include <algorithm>
#include <log4cxx/logger.h>

static auto logger = log4cxx::Logger::getLogger("dp1-fragm");

int checkFilter(const bool& switched) {
	if (switched)
		return 1;
	else
		return 0;
}

void calcNumberFrag(const unsigned int tiles_factor, const int& im_width, const int& im_height, int& numFragX , int& numFragY) {
	if (tiles_factor == 1) {
		numFragY = 1;
		numFragX = 1;
	} 
	else if (tiles_factor == 2){
		if (im_width > im_height){
			numFragX = 2;
			numFragY = 1;
		}
		else {
			numFragX = 1;
			numFragY = 2;
		}
	}
	else if (tiles_factor == 6) {
		if (im_width > im_height) {
			numFragX = 3;
			numFragY = 2;
		}
		else {
			numFragX = 2;
			numFragY = 3;
		}
	}
	else if (tiles_factor == 4) {
		numFragX = 2;
		numFragY = 2;		
	}
	else if (tiles_factor == 8) {
		if (im_width > im_height) {
			numFragX = 4;
			numFragY = 2;
		}
		else {
			numFragX = 2;
			numFragY = 4;
		}
	}
	else if (tiles_factor == 10) {
		if (im_width > im_height) {
			numFragX = 5;
			numFragY = 2;
		}
		else {
			numFragX = 2;
			numFragY = 5;
		}
	}
	else if ((tiles_factor == 12) || (tiles_factor == 14)) {
		if (im_width > im_height) {
			numFragX = 4;
			numFragY = 3;
		}
		else {
			numFragX = 3;
			numFragY = 4;
		}
	}
	else if (tiles_factor == 16) {
		numFragX = 4;
		numFragY = 4;
	}
	else if (tiles_factor == 18) {
		if (im_width > im_height) {
			numFragX = 6;
			numFragY = 3;
		}
		else {
			numFragX = 3;
			numFragY = 6;
		}
	}
	else if ((tiles_factor == 20) || (tiles_factor == 22)) {
		if (im_width > im_height) {
			numFragX = 5;
			numFragY = 4;
		}
		else {
			numFragX = 4;
			numFragY = 5;
		}
	}
	else if ((tiles_factor == 24) || (tiles_factor == 26)) {
		if (im_width > im_height) {
			numFragX = 6;
			numFragY = 4;
		}
		else {
			numFragX = 4;
			numFragY = 6;
		}
	}
	else if (tiles_factor >= 28) {
		if (im_width > im_height) {
			numFragX = 7;
			numFragY = 4;
		}
		else {
			numFragX = 4;
			numFragY = 7;
		}
	}
}

void calcBorderSize(const ns_datapro1::prg_config::cfg_one_filter& corr, const ns_datapro1::prg_config::cfg_one_filter& matched,
                    const ns_datapro1::prg_config::cfg_one_filter& blur, const int& def_border,
                    int &border_x, int &border_y) {
	int k_corr, k_matched, k_blur;
	k_corr = checkFilter(corr.switched);
	k_matched = checkFilter(matched.switched);
	k_blur = checkFilter(blur.switched);

    border_x = def_border + k_corr * int(corr.width / 2);
    border_y = def_border + k_corr * int(corr.height / 2);
//	border_x = std::max(std::max(std::max(k_corr * corr.width / 2, k_matched * matched.width / 2), k_blur * blur.width / 2), def_border);
//	border_y = std::max(std::max(std::max(k_corr * corr.height / 2, k_matched * matched.height / 2), k_blur * blur.height / 2), def_border);
}

void calcSizeFragments(const int& sizeYin, const int& sizeXin, const int& borderY, const int& borderX, const int& numFragY, const int& numFragX,
	int& sizePartY, int& sizePartX, int& sizePartYend, int& sizePartXend, int& sizeFragY, int& sizeFragX, int& sizeFragYend, int& sizeFragXend) {
	sizePartY = (int)floor(double(sizeYin) / numFragY);
	sizePartX = (int)floor(double(sizeXin) / numFragX);
	sizePartYend = sizePartY + (sizeYin - sizePartY * numFragY);
	sizePartXend = sizePartX + (sizeXin - sizePartX * numFragX);
	sizeFragY = sizePartY + 2 * borderY;
	sizeFragX = sizePartX + 2 * borderX;
	sizeFragYend = sizePartYend + 2 * borderY;
	sizeFragXend = sizePartXend + 2 * borderX;
}

void calcSizePart(const int& sizeYin, const int& sizeXin, const int& numFragY, const int& numFragX,
	int& sizePartY, int& sizePartX, int& sizePartYend, int& sizePartXend) {
	sizePartX = sizeXin / numFragX;
	sizePartY = sizeYin / numFragY;
	sizePartXend = sizePartX + (sizeXin % sizePartX);
	sizePartYend = sizePartY + (sizeYin % sizePartY);
}

void selectSize(const int& numFragY, const int& numFragX, const int& iFragY, const int& iFragX,
	const int& sizeFragY, const int& sizeFragX, const int& sizeFragYend, const int& sizeFragXend,
	int& sizeY, int& sizeX) {
	if (iFragY == (numFragY - 1))
		sizeY = sizeFragYend;
	else
		sizeY = sizeFragY;

	if (iFragX == (numFragX - 1))
		sizeX = sizeFragXend;
	else
		sizeX = sizeFragX;
}

int calcVariant(const int& numFragY, const int& numFragX, const int& iFragY, const int& iFragX) {
	// | 1 | 2 | 3 |
	// | 4 | 5 | 6 |
	// | 7 | 8 | 9 |
	// -------------
	// |10 |11 |12 |
	// ---------
	// |13 |
	// |14 |
	// |15 |
	// -----
	// _
	//|0|
	// -
	int var = 0;
	if ((numFragY == 1) && (numFragX != 1)) {
		if (iFragX == 0)
			var = 10;
		else if ((iFragX > 0) && (iFragX < (numFragX - 1)))
			var = 11;
		else if (iFragX == (numFragX - 1))
			var = 12;
	}
	else if ((numFragY != 1) && (numFragX == 1)) {
		if (iFragY == 0)
			var = 13;
		else if ((iFragY > 0) && (iFragY < (numFragY - 1)))
			var = 14;
		else if (iFragY == (numFragY - 1))
			var = 15;
	}
	else if ((numFragY == 1) && (numFragX == 1)) {
		var = 0;
	}
	else if ((numFragY != 1) && (numFragX != 1)) {
		if (iFragX == 0) {
			if (iFragY == 0)
				var = 1;
			else if ((iFragY > 0) && (iFragY < (numFragY - 1)))
				var = 4;
			else if (iFragY == (numFragY - 1))
				var = 7;
		}
		else if ((iFragX > 0) && (iFragX < (numFragX - 1))) {
			if (iFragY == 0)
				var = 2;
			else if ((iFragY > 0) && (iFragY < (numFragY - 1)))
				var = 5;
			else if (iFragY == (numFragY - 1))
				var = 8;
		}
		else if (iFragX == (numFragX - 1)) {
			if (iFragY == 0)
				var = 3;
			else if ((iFragY > 0) && (iFragY < (numFragY - 1)))
				var = 6;
			else if (iFragY == (numFragY - 1))
				var = 9;
		}
	}
	return var;
}

cv::Mat splitImage(const cv::Mat& image, const int &border_x, const int &border_y, const int& numFragX, const int& numFragY, const int &width, const int &height,
	const int &width_last_column, const int &height_last_row, const int &i, const int &j) {
	cv::Mat tmp, out;
	int x_start_rec, y_start_rec, width_rec, height_rec;
	x_start_rec = width * j;
	y_start_rec = height * i;
	width_rec = (j == (numFragX - 1)) ? width_last_column : width;
	height_rec = (i == (numFragY - 1)) ? height_last_row : height;
	//shift_x = x_start_rec - border_x; shift_y = y_start_rec - border_y;
	if((x_start_rec + width_rec) > image.cols || (y_start_rec + height_rec) > image.rows){
		LOG4CXX_ERROR(logger, "fragmentation - x_start_rec: " << x_start_rec << ", width_rec: " << width_rec << ", y_start_rec: " << y_start_rec << ", height_rec: " << height_rec);
		LOG4CXX_ERROR(logger, "image.cols: " << image.cols << ", image.rows: " << image.rows);
		LOG4CXX_ERROR(logger, "x_start_rec + width_rec: " << x_start_rec + width_rec << ", y_start_rec + height_rec: " << y_start_rec + height_rec);
	}
	cv::Rect roi(x_start_rec,
		y_start_rec,
		width_rec,
		height_rec);
	cv::copyMakeBorder(image(roi), tmp, border_y, border_y,
		border_x, border_x, cv::BORDER_REFLECT_101);
	out = tmp.clone();
	return out;
}

//std::vector<cv::Mat> splitImage2Fragments(const cv::Mat& image, const int &border_x, const int &border_y, const int& numFragX, const int& numFragY)
//{
//	// All images should be the same size ...
//	int width = image.cols / numFragX;
//	int height = image.rows / numFragY;
//	// ... except for the Mth column and the Nth row
//	int width_last_column = width + (image.cols % width);
//	int height_last_row = height + (image.rows % height);
//	int shift_x, shift_y;
//	std::vector<cv::Mat> result;
//	cv::Mat tmp, out;
//	for (int i = 0; i < numFragY; ++i){
//		for (int j = 0; j < numFragX; ++j){
//			//// Compute the region to crop from
//			//x_start_rec = width * j;
//			//y_start_rec = height * i;
//			//width_rec = (j == (numFragX - 1)) ? width_last_column : width;
//			//height_rec = (i == (numFragY - 1)) ? height_last_row : height;
//			//shift_x = x_start_rec - border_x; shift_y = y_start_rec - border_y;
//			//cv::Rect roi(x_start_rec,
//			//	y_start_rec,
//			//	width_rec,
//			//	height_rec);
//			//cv::copyMakeBorder(image(roi), tmp, border_y, border_y,
//			//	border_x, border_x, cv::BORDER_REFLECT_101);
//			//out = tmp.clone();
//			out = splitImage(image, border_x, border_y, numFragX, numFragY, width, height,
//				width_last_column, height_last_row, i, j, shift_x, shift_y);
//			result.push_back(out);
//		}
//	}
//	return result;
//}

