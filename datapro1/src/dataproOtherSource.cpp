//
// Created by user on 13.06.24.
//

#include "dataproOtherSource.h"

void choice_source(const ns_datapro1::prg_config::cfg_source_frame& source_fr, cv::VideoCapture& cap){
    if (source_fr.source ==  "webcam")
        cap.open(source_fr.deviceID, source_fr.apiID);
    else if (source_fr.source ==  "ipcam" || source_fr.source ==  "videofile" || source_fr.source ==  "imagefile" )
        cap.open(source_fr.link);
}

void read_frame(cv::VideoCapture& cap, cv::Mat& frame){
    cap >> frame;
}
