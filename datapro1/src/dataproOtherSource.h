//
// Created by user on 13.06.24.
//

#ifndef CLT_OPTIC_DATAPROOTHERSOURCE_H
#define CLT_OPTIC_DATAPROOTHERSOURCE_H
#include "datarpoTypes.h"
#include "dp1_config.h"

void choice_source(const ns_datapro1::prg_config::cfg_source_frame& source_fr, cv::VideoCapture& cap);

void read_frame(cv::VideoCapture& cap, cv::Mat& frame);

#endif //CLT_OPTIC_DATAPROOTHERSOURCE_H
