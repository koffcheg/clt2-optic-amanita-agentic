
#pragma once

#include "config.hpp"
#include "trackTest.hpp"
#include "wTrackTest.hpp"
#include "wtargetSim.hpp"
#include "utils/fileLogger.hpp"
#include <QTimer>
#include <memory>
#include <mutex>
#include <string>
#include <turretTypes.hpp>

namespace turret {

class Application : public QObject {
  Q_OBJECT
public:
  explicit Application(const Config &config);
  Application(const Application &) = delete;
  Application &operator=(const Application &) = delete;
  ~Application();
  void run();
  void terminate();

  void startTrackTest(const TrackTest &trackTest);
  void processTrackTest();
  bool trackTestEnabled() { return m_trackTestEnabled; }
  const LCSv &trackTestPos() { return m_trackTestPos; }
  void stop();

private:
  void onClose();
  void onStart();
  void onStop();
  void onUpdate();
  void onTimer();

  const Config &m_config;

  WTrackTest m_targetSettings;
  WTargetSim m_targetSim;
  FileLogger m_flog;

  bool m_trackTestEnabled = false;
  TrackTest m_trackTest{};
  LCSv m_trackTestPos{};


  QTimer m_timer;
};

} // namespace turret