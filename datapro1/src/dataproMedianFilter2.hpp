/*
 * MedianFilter.hpp
 *
 * Class to implement a pixelwise median filter.
 */

#ifndef __MEDIAN_FILTER_HPP2__
#define __MEDIAN_FILTER_HPP2__

#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include <deque>
#include <algorithm>
#include <tuple>

class MedianFilter2 {
public:
    // Перелік для вибору типу вихідного кадру
    enum class OutputMode {
        MEDIAN,      // Повертати медіанний кадр
        DIFFERENCE   // Повертати різницю між першим кадром та медіанним
    };

private:
    struct Pixel {
        uchar gray;
        bool operator < (const Pixel& p) const { return gray < p.gray; }
    };

    int filter_length_;  // Загальна довжина фільтра
    int N_frame_;        // Кількість кадрів для обчислення медіани
    int m_;              // Крок між кадрами (filter_length = N_frame * m + 1)
    bool add_to_end_;    // Напрямок додавання нових кадрів (true - в кінець, false - на початок)
    bool use_affine_;    // Чи застосовувати афінні перетворення
    OutputMode output_mode_; // Режим виведення результату

    std::deque<cv::Mat> frame_buffer_;           // Буфер оригінальних кадрів
    std::vector<cv::Mat> transformed_frames_;    // Буфер трансформованих кадрів
    std::vector<cv::Mat> affine_matrices_;       // Матриці афінних перетворень

    bool is_init_;                               // Прапорець ініціалізації
    int max_features_;                           // Макс. кількість особливих точок для афінного перетворення
    float ransac_threshold_;                     // Поріг для RANSAC при пошуку трансформації

public:
    MedianFilter2(int N_frame, int m, bool add_to_end = true,
                 bool use_affine = false,
                 OutputMode output_mode = OutputMode::DIFFERENCE,
                 int max_features = 500, float ransac_threshold = 3.0);

    cv::Mat process_frame(const cv::Mat &frame);

    // Допоміжні методи
    void init_buffer(const cv::Mat &first_frame);
    std::vector<int> get_frame_indices();
    cv::Mat compute_affine_transform(const cv::Mat &src, const cv::Mat &dst);
    void update_affine_transforms();
    void apply_affine_transforms();
    std::vector<uchar> collect_pixel_values_affine(int row, int col);
    std::vector<uchar> collect_pixel_values_direct(int row, int col);
    cv::Mat compute_median_frame(const cv::Mat &frame);

    // Гетери та сетери
    void set_use_affine(bool use_affine) { use_affine_ = use_affine; }
    [[nodiscard]] bool get_use_affine() const { return use_affine_; }
    void set_output_mode(OutputMode mode) { output_mode_ = mode; }
    [[nodiscard]] OutputMode get_output_mode() const { return output_mode_; }
};

#endif
