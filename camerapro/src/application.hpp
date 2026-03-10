
#pragma once

#include "camera.hpp"
#include "camera_frame.hpp"
#include "cp_config.h"
#include "cp_ipc_cam2dp1_if.h"
#include "turretInfoClient.hpp"
#include "frameDisplay.hpp"
#include <memory>
#include <string>

namespace cam_pro {

class Application {
public:
  explicit Application(const Config &config);
  ~Application();
  int run();

  bool terminated() { return !m_run; };
  void terminate() { m_run = false; };

private:
  void fillFrame(Frame *frame);
  void processOpen();
  void processOpened();
  void getIPC();

  const Config &m_config;

  ipc_data_tr *m_ipcDataTr = nullptr;
  // todo: std::unique_ptr<ipc_data_tr> m_ipcDataTr
  int m_frameIndex = 0;
  bool m_run = true;
  std::unique_ptr<Camera> m_camera = nullptr;
  turret::TurretInfoClient m_turret;
  Frame *m_frame = nullptr;
  FrameDisplay m_frameDisplay;
};

} // namespace cam_pro