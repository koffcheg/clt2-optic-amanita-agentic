
#include "camera.hpp"

namespace cam_pro {

Camera::Camera(const json_t *config) { loadConfig(config); }

Camera::~Camera() {}

void Camera::loadConfig(const json_t * /*config*/) {
  // todo:
}

} // namespace cam_pro
