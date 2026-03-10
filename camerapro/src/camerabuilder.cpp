

#include "camerabuilder.hpp"
#include "camera.hpp"
#include "webcamera.hpp"
#include <jansson.h>
#include <memory>

namespace cam_pro {

std::unique_ptr<Camera> buildCamera(const Config& config) {
  // todo: camera type register?
  if (config.camera.type==WebCamera::type) {
    return std::make_unique<WebCamera>(config.camera.cameraJson);
  }
  return nullptr;
}

} // namespace cam_pro
