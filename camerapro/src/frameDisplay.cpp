
#include "frameDisplay.hpp"

#include <logHelper.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

static auto logger = log4cxx::Logger::getLogger("frameDisplay");

namespace cam_pro {

FrameDisplay::FrameDisplay(bool display) : m_display(display) {
  if(m_display)
  cv::namedWindow(m_windowName, cv::WINDOW_AUTOSIZE);
}

void FrameDisplay::display(Frame &frame) {
  if (m_display) {
    const cv::Mat tmp = frame.asMat();
    cv::imshow(m_windowName, tmp);
    cv::pollKey();
  }
  /*todo:
  if(video_save)
    video.write(tmp_frame);*/
}

FrameDisplay::~FrameDisplay() { /*cv::destroyWindow(m_windowName);*/ }

} // namespace cam_pro
