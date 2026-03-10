//
// Created by user on 04.07.24.
//
#include "dataproContourOperations.h"

bool RectIntersect(const cv::Rect& r1, const  cv::Rect& r2){
    if(r1.x < r2.x + r2.width &&
    r1.x + r1.width > r2.x &&
    r1.y <  r2.y + r2.height &&
    r1.y + r1.height > r2.y)
        return true;
    else
        return false;
}

std::vector<cv::Rect> calc_rect_border(const int &border_x, const int &border_y,
                        const int &width, const int &height,
                        const int &width_last_column, const int &height_last_row,
                        const int& numFragX, const int& numFragY,
                        const int &i, const int &j){
    std::vector<cv::Rect> rect(4);
    int width_rec = (j == (numFragX - 1)) ? width_last_column : width;
    int height_rec = (i == (numFragY - 1)) ? height_last_row : height;
    // up border
    rect[0].x = -border_x; rect[0].y = -border_y;
    rect[0].width = width_rec + 2 * border_x; rect[0].height = 2 * border_y;
    // left border
    rect[1].x = -border_x; rect[1].y = border_y;
    rect[1].width = 2 * border_x; rect[1].height = height_rec - 2 * border_y;
    // right border
    rect[2].x = width_rec - border_x; rect[2].y = border_y;
    rect[2].width = 2 * border_x; rect[2].height = height_rec - 2 * border_y;
    // down border
    rect[3].x = - border_x; rect[3].y = height - border_y;
    rect[3].width = width_rec + 2 * border_x; rect[3].height = 2 * border_y;
    return rect;
}

void shift_rect_border(const int &i, const int &j, const int &width, const int &height, std::vector<cv::Rect>& rect){
    int shift_x = width * j, shift_y = height * i;
    cv::Point shift(shift_x, shift_y);
    rect[0] += shift; rect[1] += shift; rect[2] += shift; rect[3] += shift;
}

std::vector<int> index_contours_on_border(const std::vector<TDrawMeasurement>& rect,
                                  const int &border_x, const int &border_y,
                                  const int &width, const int &height,
                                  const int &width_last_column, const int &height_last_row,
                                  const int& numFragX, const int& numFragY,
                                  const int &i, const int &j){
    std::vector<int> ind_contours_on_border;
    std::vector<cv::Rect> rect_border = calc_rect_border(border_x, border_y,
                                                     width, height,
                                                     width_last_column, height_last_row,
                                                     numFragX, numFragY,
                                                     i, j);
    shift_rect_border(i, j, width, height, rect_border);
    for(int count = 0; count < (int)rect.size(); count++){
        if (RectIntersect(rect[count].rect, rect_border[0]) ||
            RectIntersect(rect[count].rect, rect_border[1]) ||
            RectIntersect(rect[count].rect, rect_border[2]) ||
            RectIntersect(rect[count].rect, rect_border[3])) {
            ind_contours_on_border.push_back(count);
        }
    }
    return ind_contours_on_border;
}

std::vector<std::vector<int>> contours_on_border_all_frag(const std::vector<std::vector<TDrawMeasurement>>& vec_rect,
//                                      const std::vector<std::vector<std::vector<cv::Point>>>& vec_contours,
                                      const int &border_x, const int &border_y,
                                      const int &width, const int &height,
                                      const int &width_last_column, const int &height_last_row,
                                      const int& numFragX, const int& numFragY){
    std::vector<std::vector<int>> vec_index(numFragX*numFragY);
    std::vector<int> index;
    int count_frag;
    for(int y_frag = 0; y_frag < numFragY; y_frag++)
        for(int x_frag = 0; x_frag < numFragX; x_frag++){
            count_frag = x_frag + numFragX * y_frag;
            index = index_contours_on_border(vec_rect[count_frag],
                                                 border_x, border_y,
                                                 width, height,
                                                 width_last_column, height_last_row,
                                                 numFragX, numFragY,
                                                 y_frag, x_frag);
            vec_index[count_frag] = index;
        }
    return vec_index;
//    for(int i = 0; i < numFragX*numFragY; i++)
//        for(int i_cont = 0; i_cont < (int)vec_index[i].size(); i_cont++)
//        vec_index[i];
}

void choice_contours_on_border(const std::vector<std::vector<int>>& vec_index,
                               const std::vector<std::vector<std::vector<cv::Point>>>& vec_contours,
                               const std::vector<std::vector<TDrawMeasurement>>& vec_rect,
                               std::vector<std::vector<cv::Point>>& contours_frame,
                               std::vector<std::vector<std::vector<cv::Point>>>& vec_contours_border,
                               std::vector<TDrawMeasurement>& rect_frame,
                               std::vector<std::vector<TDrawMeasurement>>& vec_rect_border){
    std::vector<std::vector<cv::Point>> tmp_contr;
    std::vector<std::vector<TDrawMeasurement>> tmp_rect;
//    for()

}