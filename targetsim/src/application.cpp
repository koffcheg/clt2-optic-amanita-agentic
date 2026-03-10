
#include "application.hpp"
#include "utils.hpp"
#include <QWidget>
#include <chrono>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <thread>

namespace turret {

Application::Application(const Config &config)
    : m_config(config),
      m_targetSim(m_config.field.width, m_config.field.height),
      m_flog(m_config.flog.filename, "date time az el vaz vel aaz ael",
             m_config.flog.enabled) {
  connect(&m_targetSettings, &WTrackTest::onClose, this, &Application::onClose);
  connect(&m_targetSettings, &WTrackTest::onStart, this, &Application::onStart);
  connect(&m_targetSettings, &WTrackTest::onStop, this, &Application::onStop);
  connect(&m_targetSettings, &WTrackTest::onUpdate, this,
          &Application::onUpdate);
  connect(&m_timer, &QTimer::timeout, this, &Application::onTimer);

  m_targetSim.addTarget(std::make_unique<Target>(0, 0, m_config.target.radius));
}

Application::~Application() {}

void Application::onClose() { m_targetSim.close(); }

void Application::onStart() {
  m_trackTest.startTrack(turret::DateTimeClock::now());
  m_timer.start(m_config.simulator.deltaTms);
}

void Application::onStop() { m_timer.stop(); }

void Application::onUpdate() { m_trackTest = m_targetSettings.trackTest(); }

void Application::onTimer() {
  LCSv &p = m_trackTestPos;

  p = m_trackTest.track(turret::DateTimeClock::now());
  for (size_t i = 0; i < m_targetSim.count(); i++) {
    m_targetSim.targets(i).x = p.alpha;
    m_targetSim.targets(i).y = p.beta;
  }
  m_targetSim.update();
  m_flog.log(p.time, {p.alpha, p.beta, p.alphaSpeed, p.betaSpeed,
                      p.alphaAcceleration, p.betaAcceleration});
}

void Application::run() {
  m_targetSettings.show();
  m_targetSim.showFullScreen();
}

void Application::terminate() {}

void Application::startTrackTest(const TrackTest &trackTest) {
  m_trackTest = trackTest;
  m_trackTestEnabled = true;
  m_trackTest.startTrack(turret::DateTimeClock::now());
}

void Application::processTrackTest() {
  if (m_trackTestEnabled) {
    m_trackTestPos = m_trackTest.track(DateTimeClock::now());
  }
}

void Application::stop() { m_trackTestEnabled = false; }

} // namespace turret