#ifndef DATAPRO_FILTER_H
#define DATAPRO_FILTER_H
#define _USE_MATH_DEFINES
#include <opencv2/opencv.hpp>
#include <cmath>
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
void filter2DCovariance(const cv::Mat& img, const cv::Mat& kernel, cv::Mat& dst);
void filter2DMatched(const cv::Mat& img, const int& length_stroke_x, const int& length_stroke_y, cv::Mat& dst);
void inverseMedian2DFilter(const cv::Mat& img, const int& size_kernel_1, const int& size_kernel_2, const bool& rejection, cv::Mat& dst);
void createKernelStroke(const int& length_stroke_x, const int& length_stroke_y, cv::Mat& dst);
void createKernelGaussCov(const int& size_kernel, cv::Mat& dst);
#endif
