
#pragma once

#include "config.hpp"
#include "trackTest.hpp"
#include "turretControlClient.hpp"
#include "turretInfoClient.hpp"
#include <memory>
#include <mutex>
#include <string>
#include <turretTypes.hpp>

namespace turret {

class Application {
public:
  explicit Application(const Config &config);
  Application(const Application &) = delete;
  Application &operator=(const Application &) = delete;
  ~Application();
  void run();
  void terminate();

  TurretInfoClient &infoClient() { return m_infoClient; };
  TurretControlClient &controlClient() { return m_controlClient; };

  bool trackObject() { return m_trackObject; }
  uint32_t trackObjectId() { return m_trackObjectId; }

  void startTrackObject(int32_t trackObjectId);
  void setTrackObjectCorrections(double az, double el);
  void trackObjectCorrections(double &az, double &el);
  void startTrackTest(const TrackTest &trackTest);
  void processTrackTest();
  bool trackTestEnabled() { return m_trackTestEnabled; }
  const LCSv &trackTestPos() { return m_trackTestPos; }
  void stop();

private:
  void processConnect();
  void processConnected();

  std::recursive_mutex m_mutex;

  const Config &m_config;
  TurretInfoClient m_infoClient;
  TurretControlClient m_controlClient;

  std::atomic_bool m_trackTestEnabled;
  std::atomic_bool m_trackObject;
  std::atomic_uint32_t m_trackObjectId;
  double m_trackCorrectionAz;
  double m_trackCorrectionEl;

  TrackTest m_trackTest;
  LCSv m_trackTestPos;
};

} // namespace turret