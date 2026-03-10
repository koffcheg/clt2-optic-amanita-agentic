//
// Created by user on 18.05.25.
//
#include "dataproMedianFilter2.hpp"
#include <random>

MedianFilter2::MedianFilter2(int N_frame, int m, bool add_to_end,
                           bool use_affine,
                           OutputMode output_mode,
                           int max_features, float ransac_threshold)
        : N_frame_(N_frame),
          m_(m),
          filter_length_(N_frame * m + 1),
          add_to_end_(add_to_end),
          use_affine_(use_affine),
          output_mode_(output_mode),
          is_init_(false),
          max_features_(max_features),
          ransac_threshold_(ransac_threshold)
{
    // Перевірка коректності параметрів
    if (N_frame <= 0 || m <= 0) {
        throw std::invalid_argument("N_frame і m повинні бути більше 0");
    }
}

void MedianFilter2::init_buffer(const cv::Mat &first_frame)
{
    // Очищаємо буфери
    frame_buffer_.clear();
    transformed_frames_.clear();
    affine_matrices_.clear();

    cv::Mat noise = first_frame.clone();
    // Заповнюємо буфер копіями першого кадру
    for (int i = 0; i < filter_length_; ++i) {
        cv::randu(noise,1,255);
        frame_buffer_.push_back(noise.clone());
    }

    is_init_ = true;
}

// Отримати індекси кадрів для медіанного фільтра
std::vector<int> MedianFilter2::get_frame_indices()
{
    std::vector<int> indices;
    indices.reserve(N_frame_);

    // Починаємо з другого кадру (індекс 1) і беремо кожен m-й кадр
    for (int i = 0; i < N_frame_; ++i) {
        int frame_idx = 1 + i * m_;
        if (frame_idx < (int)frame_buffer_.size()) {
            indices.push_back(frame_idx);
        }
    }

    return indices;
}

// Обчислення афінного перетворення між двома кадрами
cv::Mat MedianFilter2::compute_affine_transform(const cv::Mat &src, const cv::Mat &dst)
{
    // Виявлення особливих точок (ключових точок)
    cv::Ptr<cv::Feature2D> detector = cv::ORB::create(max_features_);
    std::vector<cv::KeyPoint> keypoints_src, keypoints_dst;
    cv::Mat descriptors_src, descriptors_dst;

    // Виявлення та обчислення дескрипторів
    detector->detectAndCompute(src, cv::noArray(), keypoints_src, descriptors_src);
    detector->detectAndCompute(dst, cv::noArray(), keypoints_dst, descriptors_dst);

    // Прив'язка дескрипторів
    cv::BFMatcher matcher(cv::NORM_HAMMING);
    std::vector<std::vector<cv::DMatch>> knn_matches;

    if (descriptors_src.empty() || descriptors_dst.empty()) {
        // Повертаємо одиничну матрицю, якщо не знайдено точок
        return cv::Mat::eye(2, 3, CV_32F);
    }

    matcher.knnMatch(descriptors_src, descriptors_dst, knn_matches, 2);

    // Фільтрація співпадінь (тест Лоу)
    std::vector<cv::DMatch> good_matches;
    const float ratio_thresh = 0.7f;
    for (const auto& matches : knn_matches) {
        if (matches.size() >= 2 && matches[0].distance < ratio_thresh * matches[1].distance) {
            good_matches.push_back(matches[0]);
        }
    }

    // Вилучення координат точок для обчислення перетворення
    std::vector<cv::Point2f> src_pts, dst_pts;
    for (const auto& match : good_matches) {
        src_pts.push_back(keypoints_src[match.queryIdx].pt);
        dst_pts.push_back(keypoints_dst[match.trainIdx].pt);
    }

    // Обчислення афінного перетворення за допомогою RANSAC
    cv::Mat affine_matrix = cv::Mat::eye(2, 3, CV_32F);
    if (src_pts.size() >= 3) {  // Потрібно мінімум 3 точки для афінного перетворення
        affine_matrix = cv::estimateAffinePartial2D(src_pts, dst_pts, cv::noArray(), cv::RANSAC, ransac_threshold_);

        // Якщо estimateAffinePartial2D не зміг знайти трансформацію, повертаємо одиничну матрицю
        if (affine_matrix.empty()) {
            affine_matrix = cv::Mat::eye(2, 3, CV_32F);
        }
    }

    return affine_matrix;
}

// Оновлення афінних перетворень для обраних кадрів
void MedianFilter2::update_affine_transforms()
{
    std::vector<int> indices = get_frame_indices();
    if (indices.empty()) return;

    // Очистити попередні матриці
    affine_matrices_.clear();
    affine_matrices_.resize(indices.size());

    // Перший кадр з вибірки - це наш еталон
    cv::Mat reference_frame = frame_buffer_[indices[0]];

    // Для першого кадру трансформація - одинична матриця
    affine_matrices_[0] = cv::Mat::eye(2, 3, CV_32F);

    // Обчислити трансформації для інших кадрів
    for (size_t i = 1; i < indices.size(); ++i) {
        cv::Mat current_frame = frame_buffer_[indices[i]];
        affine_matrices_[i] = compute_affine_transform(current_frame, reference_frame);
    }
}

// Застосування афінних перетворень до обраних кадрів
void MedianFilter2::apply_affine_transforms()
{
    std::vector<int> indices = get_frame_indices();
    if (indices.empty()) return;

    // Очистити і підготувати буфер трансформованих кадрів
    transformed_frames_.clear();
    transformed_frames_.resize(indices.size());

    // Розмір вихідного зображення (береться з першого кадру)
    cv::Size output_size = frame_buffer_[indices[0]].size();

    // Застосуванння трансформацій до кожного кадру
    for (size_t i = 0; i < indices.size(); ++i) {
        if (i == 0) {
            // Перший кадр не трансформуємо (він є еталоном)
            transformed_frames_[i] = frame_buffer_[indices[i]].clone();
        } else {
            // Застосовуємо трансформацію до поточного кадру
            cv::Mat transformed;
            cv::warpAffine(frame_buffer_[indices[i]], transformed,
                           affine_matrices_[i], output_size,
                           cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(0));
            transformed_frames_[i] = transformed;
        }
    }
}

// Збір значень пікселів з трансформованих кадрів (з врахуванням афінних перетворень)
std::vector<uchar> MedianFilter2::collect_pixel_values_affine(int row, int col)
{
    std::vector<uchar> values;
    values.reserve(transformed_frames_.size());

    for (const auto& frame : transformed_frames_) {
        // Перевірка, чи координати є в межах зображення
        if (row >= 0 && row < frame.rows && col >= 0 && col < frame.cols) {
            values.push_back(frame.at<uchar>(row, col));
        }
    }

    return values;
}

// Збір значень пікселів безпосередньо з оригінальних кадрів (без афінних перетворень)
std::vector<uchar> MedianFilter2::collect_pixel_values_direct(int row, int col)
{
    std::vector<uchar> values;
    std::vector<int> indices = get_frame_indices();
    values.reserve(indices.size());

    for (int idx : indices) {
        const cv::Mat& frame = frame_buffer_[idx];
        // Перевірка, чи координати є в межах зображення
        if (row >= 0 && row < frame.rows && col >= 0 && col < frame.cols) {
            values.push_back(frame.at<uchar>(row, col));
        }
    }

    return values;
}

// Обчислення медіанного кадру
cv::Mat MedianFilter2::compute_median_frame(const cv::Mat &frame)
{
    // Якщо увімкнено афінні перетворення, обчислюємо та застосовуємо їх
    if (use_affine_) {
        update_affine_transforms();
        apply_affine_transforms();
    }

    // Створюємо вихідний кадр
    cv::Mat median_frame = cv::Mat::zeros(frame.size(), CV_8UC1);

    // Обчислюємо медіанне значення для кожного пікселя
    for (int row = 0; row < frame.rows; ++row) {
        for (int col = 0; col < frame.cols; ++col) {
            // Збираємо значення пікселя з кадрів, з урахуванням режиму роботи
            std::vector<uchar> pixel_values;
            if (use_affine_) {
                pixel_values = collect_pixel_values_affine(row, col);
            } else {
                pixel_values = collect_pixel_values_direct(row, col);
            }

            // Якщо достатньо значень, знаходимо медіану
            if (!pixel_values.empty()) {
                size_t mid = pixel_values.size() / 2;
                std::nth_element(pixel_values.begin(), pixel_values.begin() + mid, pixel_values.end());
                median_frame.at<uchar>(row, col) = pixel_values[mid];
            } else {
                // Якщо щось пішло не так, просто копіюємо поточний піксель
                median_frame.at<uchar>(row, col) = frame.at<uchar>(row, col);
            }
        }
    }

    return median_frame;
}

cv::Mat MedianFilter2::process_frame(const cv::Mat &frame)
{
    // Перевірка вхідного кадру
    if (frame.type() != CV_8UC1) {
        throw std::invalid_argument("Вхідний кадр повинен бути одноканальним (CV_8UC1)");
    }

    // Ініціалізація буфера при першому виклику
    if (!is_init_) {
        init_buffer(frame);
    }

    // Додаємо новий кадр і видаляємо найстаріший
    if (add_to_end_) {
        // Додаємо в кінець
        frame_buffer_.push_back(frame.clone());
        frame_buffer_.pop_front();
    } else {
        // Додаємо на початок
        frame_buffer_.push_front(frame.clone());
        frame_buffer_.pop_back();
    }

    // Обчислюємо медіанний кадр
    cv::Mat median_frame = compute_median_frame(frame);

    // Повертаємо медіанний кадр або різницю залежно від обраного режиму
    if (output_mode_ == OutputMode::MEDIAN) {
        return median_frame;
    } else { // OutputMode::DIFFERENCE
        // Обчислюємо різницю між першим кадром у списку та медіанним
        cv::Mat difference;
        cv::absdiff(frame_buffer_[0], median_frame, difference);
        return difference;
    }
}