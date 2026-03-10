
#include "application.hpp"
#include "utils.hpp"
#include <chrono>
#include <cmath>
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <iostream>
#include <stdexcept>
#include <thread>

namespace turret {

void moveCursorTo(int x, int y) {
  std::cout << "\033[" << y << ";" << x << "H";
}

void clearScreen() { std::cout << "\033[2J\033[H"; }

void hideCursor() { std::cout << "\033[?25l"; }

void showCursor() { std::cout << "\033[?25h"; }

Application::Application(const std::string &infoHost,
                         const std::string &infoPort)
    : m_infoClient(infoHost, infoPort) {}

Application::~Application() {}

void Application::processConnect() {
  if (m_screen != 1) {
    m_screen = 1;
    clearScreen();
    std::cout << "Connecting..." << std::endl;
  }
}

void printAxis(const AxisState &axisState) {
  using namespace std;
  cout << fmt::format("  position      = {:.2f}°   \n",
                      u::rad2deg(axisState.position));
  cout << fmt::format("  speed         = {:.2f}°/s   \n",
                      u::rad2deg(axisState.speed));
  cout << fmt::format("  error         = {:.2f}°   \n",
                      u::rad2deg(axisState.error));
  // todo: colorize output
  cout << fmt::format("  drive error   = {}\n", axisState.driveError);
  cout << fmt::format("  encoder error = {}\n", axisState.encoderError);
  cout << fmt::format("  min limit     = {}\n", axisState.minLimit);
  cout << fmt::format("  max limit     = {}\n", axisState.maxLimit);
  cout << fmt::format("  mode          = {:8}\n", toString(axisState.mode));
}

void Application::processConnected() {
  using namespace std;
  const turret::TurretState &state = m_infoClient.state();

  if (m_infoClient.stateValid()) {
    if (state.online) {
      if (m_screen != 4) {
        clearScreen();
        m_screen = 4;
      } else {
        moveCursorTo(1, 1);
      }

      cout << "turret online            \n";
      cout << fmt::format("time: {}\n", state.time);
      cout << "alpha:\n";
      printAxis(state.alpha);
      cout << "beta:\n";
      printAxis(state.beta);
      cout.flush();

    } else {
      if (m_screen != 3) {
        clearScreen();
        cout << "turret offline" << endl;
        m_screen = 3;
      }
    }

  } else {
    if (m_screen != 2) {
      clearScreen();
      cout << "Turret state is outdated" << endl;
      m_screen = 2;
    }
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

int Application::run() {
  m_infoClient.run();
  hideCursor();
  clearScreen();

  try {
    while (m_run) {
      if (!m_infoClient.connected()) {
        processConnect();
      } else {
        processConnected();
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    showCursor();
    m_infoClient.terminate();
  } catch (...) {
    showCursor();
    m_infoClient.terminate();
    throw;
  }

  return 0;
}

} // namespace turret