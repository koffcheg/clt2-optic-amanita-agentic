

#pragma once

#include "camera.hpp"
#include "cp_config.h"
#include <memory>

namespace cam_pro {

std::unique_ptr<Camera> buildCamera(const Config &config);

}
