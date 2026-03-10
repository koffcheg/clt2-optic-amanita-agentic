#include "turretBuilder.hpp"
#include "turretSim.hpp"
#include "turretSitech.hpp"
#include <logHelper.hpp>


namespace turret {

static auto logger = log4cxx::Logger::getLogger("builder");

TurretPtr buildTurret(const Config &config) {
  if (config.turret.type == "sitech") {
    return std::make_unique<TurretSitech>(config.turret.params);
  } else if (config.turret.type == "simulator") {
    return std::make_unique<TurretSim>(config.turret.params);
  } else {
    LOG_FATAL("Unknown turret type");
  }
  return nullptr;
}

} // namespace turret
