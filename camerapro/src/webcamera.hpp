#pragma once

#include "camera.hpp"
#include <memory>
#include <opencv2/opencv.hpp>
#include <string>

namespace cam_pro {
class WebCamera : public Camera {
public:
  static const std::string type;

  // ctors
  explicit WebCamera(const json_t *config);

  ~WebCamera() override;

  WebCamera(const WebCamera &) = delete;
  WebCamera operator=(const WebCamera &) = delete;
  WebCamera(WebCamera &&) = delete;
  WebCamera operator=(WebCamera &&) = delete;

  /// @brief open the camera
  void open() override;
  /// @brief close the camera
  void close() override;

  /// @brief Start live
  void start() override;

  /// @brief stop live
  void stop() override;

  /// @brief
  /// @param buffer
  bool getImage(void *buffer, int bufferSize) override;

  double desiredFps() { return m_desiredFps; }

private:
  // private methods
  bool saveImage(const cv::Mat &mat, void *buffer, int bufferSize);
  void loadConfig(const json_t *config);
  void customGray(const cv::Mat& src, cv::Mat& dst);
  void printProperties(const char* prompt);

  // private data
  int m_id = 0;
  double m_desiredFps = 0;
  double m_rawFps = 0;
  int m_divider = 0;
  int m_dividerCounter = 0;
  bool m_temporalBinning = false;
  bool m_flipV = false;
  bool m_flipH = false;

  struct {
    bool customGray= false;
    float r = 0.2989;
    float g = 0.5870;
    float b = 0.1140;
    float scale = 256;
  } m_grayScale;

  struct Config{
    std::string fourcc;
    std::vector<std::pair<int, double>> props;
  } m_config;
  static const std::vector<std::pair<int, const char *>> m_props;


  cv::Mat m_img;        // matrix to load the frames
  cv::Mat m_imgGray;    // matrix for grayscaled image
  cv::Mat m_imgGray16;  // matrix for grayscaled image (16-bit)
  cv::Mat m_imgBinning; // matrix for temporal binning
  std::unique_ptr<cv::VideoCapture> m_cap; // object to capture frames
};

} // namespace cam_pro
