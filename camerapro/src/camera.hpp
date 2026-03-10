

#pragma once

#include "utils/ticTac.hpp"
#include <functional>
#include <m_json_cfg_reader.h>
#include <string>

namespace cam_pro {
using OnCamera = std::function<void(int, int)>;
class Camera {
public:
  // ctors

  Camera() = delete;
  explicit Camera(const json_t *config);

  virtual ~Camera();
  // prohibit Camera copy and move?
  Camera(const Camera &) = delete;
  Camera &operator=(const Camera &) = delete;
  Camera(Camera &&) = delete;
  Camera &operator=(Camera &&) = delete;

  /// @brief open the camera
  virtual void open() = 0;
  /// @brief close the camera
  virtual void close() = 0;

  /// @brief Start frame grabbing
  virtual void start() = 0;
  virtual void stop() = 0;

  int width() { return m_width; }
  int height() { return m_height; }
  int getDepth() { return m_depth; }
  int bytesPerPixel() { return m_bytesPerPixel; }
  double fps() { return m_fps; }

  /// @brief Reads image from the camera and stores it to the buffer. Buffer
  /// @param buffer  a pointer to properly alligned buffer for image storage.
  /// Size of the buffer must be equal to pixelCount*bytesPerPixel
  /// @param pixelCount buffer length in pixel (width*height)
  /// @param bytesPerPixel bytes per pixel

  /// @return true if image was received,
  virtual bool getImage(void *buffer, int bufferSize) = 0; // todo????

  void setOnImage() {}

  bool opened() { return m_opened; }
  double msFrameReceived() { return m_ticTac.tac<std::milli>(); }
  double msFromLastFrame() { return m_msFromLastFrame; }

protected:
  int m_width = 0;
  int m_height = 0;
  int m_depth = 0;         /// bits per pixel
  int m_bytesPerPixel = 0; /// bytes per pixel
  double m_fps = 0;
  bool m_opened = false;

  TicTacTimer m_ticTac;
  double m_msFromLastFrame;

private:
  void loadConfig(const json_t *config);
};

} // namespace cam_pro
