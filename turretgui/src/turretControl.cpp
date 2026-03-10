
#include "turretControl.hpp"
#include "datapro2/src/coordinateTransformation.h"
#include "tg_rc_from_dp2.h"
#include "turretControlClient.hpp"
#include "utils.hpp"
#include "utils/fileLogger.hpp"
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <log4cxx/logger.h>
#include <logHelper.hpp>
#include <map>
#include <memory>

// #include <functional>

// todo: enabling....
FileLogger fileLog("dp2.txt", "date time Az El dAz dEl vAz vEl aAz aEl dt", true);

static auto logger = log4cxx::Logger::getLogger("turretControl");

// class TurretControl {
// public:
//   TurretControl(turret::Application &application);
//
//   void _on_new_tracks(int dp2_id, const std::vector<Trajectory> &tracks);
//   void _on_upd_tracks(int dp2_id, const std::vector<Trajectory> &tracks);
//   void _on_drop_tracks(int dp2_id, const std::vector<unsigned long> &tracks);
//
// private:
//   turret::Application &m_application;
// };
// TurretControl::TurretControl(turret::Application &application)
//     : m_application(application) {}

struct TurretControl {
  // std::map<>;
  turret::Application &application;
  turret::TurretControlClient &controlClient;

  double vAz = 0;
  double vEl = 0;
  double aAz = 0;
  double aEl = 0;
  double lastAz = 0;
  double lastEl = 0;
  DateTime lastT{};
};

std::unique_ptr<TurretControl> turretControl;

static void _on_new_tracks(int /*dp2_id*/, unsigned int d1nd2_proc_time_ms,
                           const std::vector<Trajectory> & /*tracks*/) {
  // Warning: is called in the context of the server thread

  // LOG4CXX_DEBUG(logger, "dp2-id: " << dp2_id << ", size: " << tracks.size());
  // for (const auto &el : tracks)
  //   LOG4CXX_DEBUG(logger, "tr-id: " << el.id);
}

/*
static void _estimateSpeed(DateTime time, double Az, double El, double &VAz, double &VEl, double &aAz, double &aEl) {
  double dt = std::chrono::duration<double>(time - turretControl->lastT).count();
  if (dt < 1) { // less than a second
    double vAz = (Az - turretControl->lastAz) / dt;
    double vEl = (El - turretControl->lastEl) / dt;
    double tau = 1;
    vAz = (turretControl->vAz * tau + vAz) / (tau + 1);
    vEl = (turretControl->vEl * tau + vEl) / (tau + 1);

    aAz = (vAz - turretControl->vAz) / dt;
    aEl = (vEl - turretControl->vEl) / dt;

    aAz = (turretControl->aAz * tau + aAz) / (tau + 1);
    aEl = (turretControl->aEl * tau + aEl) / (tau + 1);

    turretControl->vAz = vAz;
    turretControl->vEl = vEl;
    turretControl->aAz = aAz;
    turretControl->aEl = aEl;
  }

  VAz = turretControl->vAz;
  VEl = turretControl->vEl;
  aAz = turretControl->aAz;
  aEl = turretControl->aEl;
  turretControl->lastT = time;
  turretControl->lastAz = Az;
  turretControl->lastEl = El;
}
*/

static void _on_upd_tracks(int /*dp2_id*/, unsigned int d1nd2_proc_time_ms, const std::vector<Trajectory> &tracks) {
  // Warning: is called in the context of server thread
  // todo: analyze dp2
  if (turretControl->application.trackObject()) {
    for (auto &track : tracks) {
      if (turretControl->application.trackObjectId() == track.id) {
        double Az, El, VAz, VEl, aAz = 0, aEl = 0;
        auto time = std::chrono::system_clock::now();
        // extrapolationAzEl(track, time, Az, El, VAz, VEl);
        // extrapolationAzEl_V_a(track, time, Az, El, VAz, VEl, aAz, aEl);
        // extrapolationAzEl_V_binocular(track, time, Az, El, VAz, VEl);
        extrapolationAzEl_4_binocular(track, time, Az, El);
        extrapolationVazVel_4_binocular(track, time, VAz, VEl);

        // VAz *= 1E12;
        // VEl *= 1E12;

        // todo: workaround to estimate the speed from the last data points
        //_estimateSpeed(time, Az, El, VAz, VEl, aAz, aEl);

        double dAz;
        double dEl;

        turretControl->application.trackObjectCorrections(dAz, dEl);
        turretControl->controlClient.track(time, Az + dAz, El + dEl, VAz, VEl, aAz, aEl);

        if (track.measurements.size()) {
          using namespace u;
          double dt = static_cast<std::chrono::duration<double>>(time - track.measurements.back().time).count();
          LOG_DEBUG(fmt::format("t={}, Az={}, El={}, VAz={}, VEl={} dt={}", time, Az / DEG, El / DEG, VAz / DEG,
                                VEl / DEG, aAz / DEG, aEl / DEG, dt));
          fileLog.log(time, {Az / DEG, El / DEG, dAz / DEG, dEl / DEG, VAz / DEG, VEl / DEG, aAz / DEG, aEl / DEG, dt*1000});
        }
      }
    }
  }

  //  LOG4CXX_DEBUG(logger, "dp2-id: " << dp2_id << ", size: " <<
  //  tracks.size()); for (const auto &el : tracks)//
  //    LOG4CXX_DEBUG(logger, "tr-id: " << el.id)//;
}

static void _on_drop_tracks(int /*dp2_id*/, const std::vector<unsigned long> & /*tracks*/) {
  // Warning: is called in the context of server thread

  //  LOG4CXX_DEBUG(logger, "dp2-id: " << dp2_id << ", size: " <<
  //  tracks.size()); for (const auto &el : tracks)
  //    LOG4CXX_DEBUG(logger, "tr-id: " << el);
}

void turretControlInit(uint16_t port, turret::Application &application) {
  if (turretControl == nullptr) {
    turretControl = std::make_unique<TurretControl>(application, application.controlClient());
  }
  init_rc_data_from_dp2(port);
  register_dp2_upd_tracks_callback(_on_upd_tracks);
  register_dp2_new_tracks_callback(_on_new_tracks);
  register_dp2_drop_tracks_callback(_on_drop_tracks);
}

void turretControlStop() { stop_rc_data_from_dp2(); }
