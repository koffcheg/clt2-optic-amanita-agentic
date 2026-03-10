/*
 * MedianFilter.hpp
 *
 * Class to implement a pixelwise median filter.
 */

#ifndef __MEDIAN_FILTER_HPP__
#define __MEDIAN_FILTER_HPP__

#include <opencv2/opencv.hpp>

#include <algorithm>
#include <vector>


// Struct to store pixel information

struct Pixel
{
//    cv::Vec<uchar, 1> color;
    uchar gray;

    Pixel(/*cv::Vec<uchar, 1> color,*/ uchar gray) : /*color(color),*/ gray(gray) { }

    bool operator < (const Pixel& other) const { return gray < other.gray; }
};


// Class definition

class MedianFilter
{
public:

    MedianFilter(int filter_length);
    cv::Mat process_frame(const cv::Mat &frame);
    void init_median_lists(int size);

private:

    cv::Mat frame;
    int filter_length;
    int size;
    bool pop_front;
    bool is_init;

    std::vector< std::vector<Pixel> > median_lists;

};


// MedianFilter public methods

MedianFilter::MedianFilter(int filter_length)
    : filter_length(filter_length)
    , size(0)
    , is_init(false)
{ }


cv::Mat MedianFilter::process_frame(const cv::Mat &gray_frame)
{
    // Get grayscale image to do sorting with
//    cv::Mat gray_frame = frame;
//    cv::cvtColor(frame, gray_frame, cv::COLOR_BGR2GRAY);

    // Initialize the filter if we haven't yet
    if ( ! is_init )
        init_median_lists((int)gray_frame.total());

//    assert( median_lists.size() == gray_frame.total() );

    // Init iterators
//    cv::MatConstIterator_<cv::Vec<uchar, 1>> bgr_it  = frame.begin<cv::Vec<uchar, 1>>(),
//                                     bgr_end = frame.end  <cv::Vec<uchar, 1>>();

    cv::MatConstIterator_<uchar>     gray_it = gray_frame.begin<uchar>(),
            gray_end = gray_frame.end<uchar>();

    auto med_it = median_lists.begin();

    // Matrix to stuff the "current frame" into.
    cv::Mat dst(gray_frame.rows, gray_frame.cols, CV_8UC1);
    cv::MatIterator_<uchar> dst_it = dst.begin<uchar>();

    // Iterate through all pixels in each image together
    for ( ; gray_it != gray_end; /*bgr_it++,*/ gray_it++, dst_it++, med_it++)
    {
        Pixel pixel(/**bgr_it,*/ *gray_it);

        // NOTE: because quickselect partially sorts, we're not really
        // getting rid of the min/max values, but this should be fine
        if (pop_front)
            (*med_it)[0] = pixel;
        else
            (*med_it)[filter_length-1] = pixel;

        int med_idx = (*med_it).size()/2;

        std::nth_element((*med_it).begin(), (*med_it).begin()+med_idx, (*med_it).end());

        *dst_it = (*med_it)[med_idx].gray;
    }

    pop_front = ! pop_front;

    return dst;
}


void MedianFilter::init_median_lists(int size)
{
    this->size = size;

    Pixel pixel(/*0,*/ 0);
    std::vector<Pixel> vec(filter_length, pixel);
    median_lists = std::vector< std::vector<Pixel> >(size, vec);

    for (auto vec : median_lists)
        for (auto pix : vec)
            pix.gray = std::rand();

    is_init = true;
}


#endif
