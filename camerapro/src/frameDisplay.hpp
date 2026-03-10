
#pragma once

#include "camera_frame.hpp"
#include <string>

namespace cam_pro {

class FrameDisplay {
public:
  FrameDisplay(bool display);
  ~FrameDisplay();
  void display(Frame &frame);

private:
  const std::string m_windowName = "camera";
  bool m_display;
};

} // namespace cam_pro
