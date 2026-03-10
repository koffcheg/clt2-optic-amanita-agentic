#include "datarpoSegmentation.h"

std::vector<std::vector<cv::Point> > refineSegments(const cv::Mat& mask, const int& weight_element, const int& weight_height, const int& niters,
	const int& type_element, const int& level, const int& level_max, const int& threshold_type,
	const int& contour_retrieval, const int& contour_approximation, const int& shift_x, const int& shift_y){

	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::Mat temp;
	threshold(mask, temp, level, level_max, threshold_type);

	dilate(temp, temp, cv::getStructuringElement(type_element, cv::Size(weight_element, weight_height)), cv::Point(-1, -1), niters);//cv::MORPH_ELLIPSE
//    if (test.debug){
//        cv::Rect roi(data_param.border_x,
//                     data_param.border_y,
//                     data_param.sizePartX,
//                     data_param.sizePartY);
//        tmp_folder = test.out_folder + folder_name_dp1.frame_segment;
//        check_and_create_directories(tmp_folder);
//        cv::imwrite(tmp_folder + "/" + file_name + ".png", var.vec_bgmask[index](roi), compression_params);
//        var.vec_bgsubtractor[index]->getBackgroundImage	(tmp);
//        tmp_folder = test.out_folder + folder_name_dp1.frame_background;
//        check_and_create_directories(tmp_folder);
//        cv::imwrite(tmp_folder + "/" + file_name + ".png", tmp(roi), compression_params);
//    }
	findContours(temp, contours, hierarchy, contour_retrieval, contour_approximation, cv::Point(shift_x, shift_y));

	return contours;
}

std::vector<TPixelObj> definition_object_pixels(const cv::Mat& img, const std::vector<cv::Point> & contour, const cv::Rect& rect){
    std::vector<TPixelObj> vec_pixel;
    TPixelObj tmp;
    int x, y;
    for(int i=0; i<rect.height; i++)
        for(int j=0; j<rect.width; j++){
            x = j + rect.x; y = i + rect.y;
            if (cv::pointPolygonTest(contour,cv::Point2i(x,y),false)>=0){
                tmp.br = img.at<uchar>(i,j);
                tmp.coor = cv::Point2i(x,y);
                vec_pixel.push_back(tmp);
            }
        }
    return vec_pixel;
}

void calc_mean(const std::vector<TPixelObj> &vec_pixel, float& mean){
    mean = 0;
    for(auto i : vec_pixel)
        mean += i.br;
    mean /= (float)vec_pixel.size();
}

void calc_std(const std::vector<TPixelObj> &vec_pixel, const float& mean, float& std){
    std = 0;
    for(auto i : vec_pixel)
        std += (i.br - mean) * (i.br - mean);
    std /= (float)(vec_pixel.size() - 1);
    std = sqrt(std);
}

void calc_mean_std(const std::vector<TPixelObj> &vec_pixel, float& mean, float& std){
    calc_mean(vec_pixel, mean);
    calc_std(vec_pixel, mean, std);
}

void calc_weight_coor(const std::vector<TPixelObj>& vec_pixel, float& weight_x, float& weight_y){
    weight_x = 0; weight_y = 0;
    float br = 0;
    for(auto i : vec_pixel){
        weight_x += float(i.coor.x) * i.br;
        weight_y += float(i.coor.y) * i.br;
        br += i.br;
    }
    weight_x/=br;
    weight_y/=br;
}

void calc_second_order_central_moments(const std::vector<TPixelObj>& vec_pixel, const float& weight_x, const float& weight_y,
                                       float& m_02, float& m_20, float& m_11){
    m_02 = 0; m_20 = 0; m_11 = 0;
    for(auto i : vec_pixel){
        m_20 += ((float)i.coor.x - weight_x) * i.br;
        m_02 += ((float)i.coor.y - weight_y) * i.br;
        m_11 += ((float)i.coor.x - weight_x) * ((float)i.coor.y - weight_y) * i.br;
    }
}

void calc_segment_angle(const float& m_02, const float& m_20, const float& m_11, float& angle){
//    angle = 0.5 * atan(2*m_11/(m_20 - m_02))/CV_PI * 180;
    angle = float(90 * atan(2*m_11/(m_20 - m_02))/CV_PI);
}

void calc_eccentricity(const float& m_02, const float& m_20, const float& m_11, float& e_11){
    e_11 = (m_20 + m_02 - sqrt((m_20 - m_02) * (m_20 - m_02) + 4 * m_11 * m_11))/
            (m_20 + m_02 + sqrt((m_20 - m_02) * (m_20 - m_02) + 4 * m_11 * m_11));
    e_11 = sqrt(1 - e_11);
}

void calc_contour_param(const int& min_segment_size, const int& max_segment_size, const double& max_height_to_width_ratio,
                        std::vector<std::vector<cv::Point>>& contours,
                        std::vector<TDrawMeasurement>& data_draw){
    data_draw.clear();
    TDrawMeasurement tmp_draw;
    std::vector<std::vector<cv::Point>> contour_tmp;
    long unsigned int idx;
    double maxArea, ratio;
    for (idx = 0; idx < contours.size(); idx++) {
        maxArea = contourArea(contours[idx]);
        if (maxArea >= min_segment_size && maxArea < max_segment_size) {
            tmp_draw.rect = cv::boundingRect(contours[idx]);
            tmp_draw.box = cv::minAreaRect(contours[idx]);
            ratio = tmp_draw.box.size.height / tmp_draw.box.size.width;
            if ((ratio <= max_height_to_width_ratio)&(ratio >= 1/max_height_to_width_ratio)){
                data_draw.push_back(tmp_draw);
                contour_tmp.push_back(contours[idx]);
            }
        }
    }
    contours = contour_tmp;
}

std::vector<TOptionsMeasurement> calcMeasurement(const cv::Mat& img, const std::vector<std::vector<cv::Point> >& contours,
                                                 const std::vector<TDrawMeasurement>& data_draw) {
	TOptionsMeasurement tmp;
	std::vector<TOptionsMeasurement> meas;
    std::vector<TPixelObj> vec_pixel;
    long unsigned int idx;
    float mean, std, weight_x, weight_y, m_11, m_02, m_20, e_11, angle;
	for (idx = 0; idx < contours.size(); idx++) {
        vec_pixel = definition_object_pixels(img, contours[idx], data_draw[idx].rect);
        calc_mean_std(vec_pixel, mean, std);
        calc_weight_coor(vec_pixel, weight_x, weight_y);
        calc_second_order_central_moments(vec_pixel, weight_x, weight_y,m_02, m_20, m_11);
        calc_segment_angle(m_02, m_20, m_11, angle);
        calc_eccentricity(m_02, m_20, m_11, e_11);
        tmp.x_weight = weight_x;//rRect.center.x;
        tmp.y_weight = weight_y;//rRect.center.y;
        tmp.x_rec = data_draw[idx].box.center.x;
        tmp.y_rec = data_draw[idx].box.center.y;
        tmp.rec_width = data_draw[idx].box.size.width;
        tmp.rec_height = data_draw[idx].box.size.height;
        tmp.obj_angel = data_draw[idx].box.angle;
        tmp.obj_angel_moment = angle;
        tmp.obj_eccentricity = e_11;
        tmp.num_pix_obj = (int)vec_pixel.size();
        tmp.mean_brightness_obj = mean;
        tmp.std_brightness_obj = std;
        meas.push_back(tmp);
	}
	return meas;
}
