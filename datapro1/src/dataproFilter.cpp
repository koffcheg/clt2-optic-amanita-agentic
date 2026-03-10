#include "dataproFilter.h"

void createKernelGaussCov(const int& size_kernel, cv::Mat& dst) {
	float sigma = (float)(0.3*(size_kernel / 2.0 - 1) + 0.8), sigma_qrt = 1/(2*sigma * sigma), tmp_x, tmp_y;
	int x0 = (size_kernel - 1) / 2, total_num = size_kernel * size_kernel;
	auto *rawdata = new float[total_num];
	for (int y = 0; y < size_kernel; y++)
		for (int x = 0; x < size_kernel; x++) {
			tmp_x = float(x - x0); tmp_y = float(y - x0);
			*(rawdata + x + y * size_kernel) = exp(-(tmp_x*tmp_x + tmp_y * tmp_y) * sigma_qrt);
		}
	dst = cv::Mat(size_kernel, size_kernel, CV_32FC1);
	std::memcpy(dst.data, rawdata, total_num * sizeof(float));
	delete[] rawdata;
}

double calcGausModelLong(const double& exp1, const double& exp2, const double& exp3,
	const double& exp6, const double& exp7,
	const double& expin3, const double& expin6) {
	double sum = exp2 * exp(exp3*expin3*expin3)*
		(erf(exp1 * (exp6 + expin6)) + erf(exp1 * (exp7 - expin6)));
	return sum;
}

void calcSizeKernelStroke(const int& length_x, const int& length_y, const float& sigma, int& size_x, int& size_y) {
	size_x = length_x + 6 * sigma;
	size_y = length_y + 6 * sigma;
}

void createKernelStroke(const int& length_stroke_x, const int& length_stroke_y, cv::Mat& dst) {
	float sigma = 1;//, sigma_qrt = 1 / (2.0)
	int size_kernel_x = 0, size_kernel_y = 0;
	calcSizeKernelStroke(length_stroke_x, length_stroke_y, sigma, size_kernel_x, size_kernel_y);
	//float tmp_x, tmp_y;
	int x0 = (size_kernel_x - 1) / 2, y0 = (size_kernel_y - 1) / 2, total_num = size_kernel_x * size_kernel_y;
	auto *rawdata = new float[total_num];

    // d:\My work astro\??????\?????? ???????\?????? Lemur.docx
	double d, exp1, exp2, exp3, exp4, exp5, exp6, exp7, exp8, exp9, exp10, exp11, exp12,
		expin1, expin2, expin3, expin4, expin5, expin6;

	//TLongDataA LongData = pGetDataVector(VoidFittingLong.useParamIn, x, q, VoidFittingLong.DefaultParam);
	exp11 = length_stroke_x * length_stroke_x + length_stroke_y * length_stroke_y;
	d = sqrt(exp11);
	exp1 = 1 / (2 * sqrt(2) * d * sigma);
	exp2 = exp1 * M_SQRT1_2;
	exp3 = -1 / (2 * sigma * exp11);
	exp4 = length_stroke_x * y0;
	exp5 = length_stroke_y * x0;
	exp9 = length_stroke_x * x0;
	exp10 = length_stroke_y * y0;
	exp12 = 2 * (exp9 + exp10);
	exp6 = exp11 - exp12;
	exp7 = exp11 + exp12;
	exp8 = exp4 - exp5;

	for (int y = 0; y < size_kernel_y; y++)
		for (int x = 0; x < size_kernel_x; x++) {
			expin1 = length_stroke_y * x;
			expin2 = length_stroke_x * y;
			expin3 = expin1 - expin2 + exp8;
			expin4 = length_stroke_x * x;
			expin5 = length_stroke_y * y;
			expin6 = 2 * (expin4 + expin5);
			*(rawdata + x + y * size_kernel_x) = (float)calcGausModelLong(exp1, exp2, exp3, exp6, exp7, expin3, expin6);
		}
	dst = cv::Mat(size_kernel_y, size_kernel_x, CV_32FC1);
	std::memcpy(dst.data, rawdata, total_num * sizeof(float));
	delete[] rawdata;
}

void filter2DMatched(const cv::Mat& img, const int& length_stroke_x, const int& length_stroke_y, cv::Mat& dst) {
	cv::Mat kernel, tmp, result;
	createKernelStroke(length_stroke_x, length_stroke_y, kernel);
    int result_cols = img.cols - kernel.cols + 1;
    int result_rows = img.rows - kernel.rows + 1;
    result.create(result_rows, result_cols, CV_32FC1);

    img.convertTo(tmp, CV_32FC1);
	matchTemplate(img, kernel, result, cv::TM_CCORR);
    result = cv::abs(result);
    cv::normalize(result, dst, 0, 255, cv::NORM_MINMAX, CV_8UC1, cv::Mat());
}

void filter2DCovariance(const cv::Mat& img, const cv::Mat& kernel, cv::Mat& dst) {
	cv::Mat tmp, result;
	int result_cols = img.cols - kernel.cols + 1;
	int result_rows = img.rows - kernel.rows + 1;

	result.create(result_rows, result_cols, CV_32FC1);

	img.convertTo(tmp, CV_32FC1);
	matchTemplate(tmp, kernel, result, cv::TM_CCOEFF);//TM_CCOEFF_NORMED TM_CCORR
	cv::normalize(result, dst, 0, 255, cv::NORM_MINMAX, CV_8UC1, cv::Mat());
}

void filter2DMedian(const cv::Mat& img, const int& size_kernel, cv::Mat& dst) {
    cv::Mat tmp, tmp2, tmp3;
	cv::medianBlur(img, tmp3, size_kernel);
    img.convertTo(tmp,CV_32F);
    tmp3.convertTo(tmp2,CV_32F);
    tmp3 = tmp - tmp2;
    cv::normalize(tmp3, dst, 0, 255, cv::NORM_MINMAX, CV_8U, cv::Mat());
}

void filter2DMedianRejection(const cv::Mat& img, const int& size_kernel_1, const int& size_kernel_2, cv::Mat& dst) {
    cv::Mat tmp;
    int size_1, size_2;
    if (size_kernel_1 < size_kernel_2){
        size_1 = size_kernel_1; size_2 = size_kernel_2;
    }
    else{
        size_2 = size_kernel_1; size_1 = size_kernel_2;
    }
    cv::medianBlur(img, tmp, size_1);
    cv::medianBlur(img, dst, size_2);
    dst = img - tmp + dst;
}

void filter2DMedianBandpass(const cv::Mat& img, const int& size_kernel_1, const int& size_kernel_2, cv::Mat& dst) {
    cv::Mat tmp, tmp2, tmp3;
    int size_1, size_2;
    if (size_kernel_1 < size_kernel_2){
        size_1 = size_kernel_1; size_2 = size_kernel_2;
    }
    else{
        size_2 = size_kernel_1; size_1 = size_kernel_2;
    }
    cv::medianBlur(img, tmp, size_1);
    cv::medianBlur(img, dst, size_2);
    tmp.convertTo(tmp2,CV_32F);
    dst.convertTo(tmp3,CV_32F);
    tmp = tmp2 - tmp3;
    cv::normalize(tmp, dst, 0, 255, cv::NORM_MINMAX, CV_8U, cv::Mat());
}

void inverseMedian2DFilter(const cv::Mat& img, const int& size_kernel_1, const int& size_kernel_2, const bool& rejection, cv::Mat& dst){
    if(size_kernel_2 == 0)
        filter2DMedian(img,size_kernel_1,dst);
    else
        if (rejection)
            filter2DMedianRejection(img, size_kernel_1, size_kernel_2,dst);
        else
            filter2DMedianBandpass(img, size_kernel_1, size_kernel_2, dst);
}