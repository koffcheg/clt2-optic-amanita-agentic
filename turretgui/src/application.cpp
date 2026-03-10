
#include "application.hpp"
#include "utils.hpp"
#include <chrono>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <thread>

namespace turret {

Application::Application(const Config &config)
    : m_config(config), m_infoClient(m_config.infoHost, m_config.infoPort),
      m_controlClient(m_config.controlHost, m_config.controlPort) {}

Application::~Application() { terminate(); }

void Application::run() {
  m_infoClient.run();
  m_controlClient.run();
}

void Application::terminate() {
  m_infoClient.terminate();
  m_controlClient.terminate();
}

void Application::startTrackObject(int32_t trackObjectId) {
  m_trackTestEnabled = false;
  m_trackObjectId = trackObjectId;
  m_trackObject = true;
}

void Application::setTrackObjectCorrections(double az, double el) {
  std::lock_guard<std::recursive_mutex> guard(m_mutex);
  m_trackCorrectionAz = az;
  m_trackCorrectionEl = el;
}

void Application::trackObjectCorrections(double &az, double &el) {
  std::lock_guard<std::recursive_mutex> guard(m_mutex);
  az = m_trackCorrectionAz;
  el = m_trackCorrectionEl;
}

void Application::startTrackTest(const TrackTest &trackTest) {
  std::lock_guard<std::recursive_mutex> guard(m_mutex);
  m_trackObject = false;
  m_trackTest = trackTest;
  m_trackTestEnabled = true;
  m_trackTest.startTrack(turret::DateTimeClock::now());
}

void Application::processTrackTest() {
  if (m_trackTestEnabled && m_controlClient.connected()) {
    m_trackTestPos = m_trackTest.track(DateTimeClock::now());
    m_controlClient.track(
        m_trackTestPos.time, m_trackTestPos.alpha, m_trackTestPos.beta,
        m_trackTestPos.alphaSpeed, m_trackTestPos.betaSpeed,
        m_trackTestPos.alphaAcceleration, m_trackTestPos.betaAcceleration);
  }
}

void Application::stop() {
  m_trackTestEnabled = false;
  m_trackObject = false;
  if (controlClient().connected()) {
    controlClient().stop();
  }
}

} // namespace turret