
#include "application.hpp"
#include "camerabuilder.hpp"
#include <chrono>
#include <cmath>
#include <fmt/format.h>
#include <iostream>
#include <logHelper.hpp>
#include <stdexcept>
#include <thread>

namespace cam_pro {

static auto logger = log4cxx::Logger::getLogger("application");

Application::Application(const Config &config)
    : m_config(config), m_camera(buildCamera(config)),
      m_turret(config.turret.address, config.turret.port,
               config.turret.reconnectIntervalms, config.turret.dataTimeotms),
      m_frameDisplay(m_config.display.display) {
  if (!m_camera) {
    throw std::runtime_error("Could not create camera class. Check config");
  }
}

Application::~Application() {
  // release unused frame
  /*
  if (m_frame) {
    // m_frame->header.isOk = false;
    // todo:
    m_frame->header.width = 0;
    m_frame->header.height = 0;
    if (m_ipcDataTr) {
      m_ipcDataTr->tr_formed_frame();
    }
  }*/
}

void Application::fillFrame(Frame *frame) {
  if (m_camera) {
    // todo: move (at least partly) to corresponding camera
    frame->header.index = m_frameIndex;
    frame->header.width = m_camera->width();
    frame->header.height = m_camera->height(); // image height in pixels
    frame->header.bytesPerPixel = m_camera->bytesPerPixel(); // pixel format
    frame->header.bitDepth = m_camera->getDepth(); // Bits per pixel per channel
    // frame->header.cameraName = ""; //Чи ми цього потребуємо?

    if (m_camera->fps() > 0) {
      frame->header.exposureLength = 1.0 / m_camera->fps(); // s, todo:
    } else {
      frame->header.exposureLength = 0; // s, todo:
    }
    using namespace std::chrono;
    auto as_duration = duration_cast<steady_clock::duration>(duration<double>(
        frame->header.exposureLength +
        (float)m_config.camera.delayms / 1000.0)); // todo: add same delta
    frame->header.exposureStart = DateTimeClock::now() - as_duration; // todo:

    frame->header.focalLength = 0; // m
    frame->header.pixelWidth = 0;  // m
    frame->header.pixelHeight = 0; // m
    frame->header.turretInfoValid = m_turret.stateValid();
    if (m_turret.stateValid()) {
      // todo: add camera position?
      auto state = m_turret.state(frame->header.exposureStart);
      frame->header.Azimuth = state.alpha.position;
      frame->header.Elevation = state.beta.position;
      frame->header.AzimuthSpeed = state.alpha.speed;
      frame->header.ElevationSpeed = state.beta.speed;
    } else {
      frame->header.Azimuth = 0;
      frame->header.Elevation = 0;
      frame->header.AzimuthSpeed = 0;
      frame->header.ElevationSpeed = 0;
    }
  }
}

void Application::getIPC() {
  if (!m_camera || !m_camera->opened()) {
    return;
  }

  if (!m_ipcDataTr) {
    m_ipcDataTr = get_ipc_data_forwarder(Frame::size(m_camera->width(),
                                                     m_camera->height(),
                                                     m_camera->bytesPerPixel()),
                                         m_config);
  } else {
    // todo: check if server width*height coincide: if (m_srv->)
  }
}

void Application::processOpen() {
  if (m_camera) {
    m_camera->open();
    if (m_camera->opened()) {
      getIPC();
    } else {
      // Wait before the next attempt
      std::this_thread::sleep_for(
          std::chrono::milliseconds(m_config.camera.reconnectWait));
    }
  }
}

void Application::processOpened() {
  if (!m_frame) {
    m_frame = static_cast<Frame *>(m_ipcDataTr->get_prt_next_frame());
  }
  if (m_frame) {
    // todo:
    m_frame->header.width = m_camera->width();
    m_frame->header.height = m_camera->height();
    m_frame->header.bytesPerPixel = m_camera->bytesPerPixel();

    if (m_camera->getImage(m_frame->data(), m_frame->dataSize())) {
      fillFrame(m_frame);
      m_frameDisplay.display(*m_frame);
      LOG_DEBUG(fmt::format("Sending frame # {}: {}x{}={} (Tproc={:.3f}ms, 1/fps={:.3f}ms)", m_frameIndex,
                            m_frame->header.width, m_frame->header.height,
                            m_frame->size(),
                            m_camera->msFrameReceived(),
                            m_camera->msFromLastFrame()));
      m_ipcDataTr->tr_formed_frame();
      m_frameIndex++;
      m_frame = nullptr;
    } else {
      // Free the frame
      // todo:  m_srv->makeEmpty(frame);
    }
  } else {
    LOG_WARN("No free IPC frames found!");
    std::this_thread::sleep_for(
        std::chrono::milliseconds(m_config.ipc.retryWait));
    // todo: skip old frames???
  }
}

int Application::run() {
  m_turret.run();
  while (m_run && m_camera) {
    if (!m_camera->opened()) {
      // Try to open the camera
      processOpen();
    } else {
      // Try to get the image from the camera
      processOpened();
    }
  }

  return 0;
}

} // namespace cam_pro