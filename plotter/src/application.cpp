
#include "application.hpp"
#include "utils.hpp"
#include <chrono>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <thread>

namespace turret {

Application::Application(const Config &config) : m_config(config) {}

Application::~Application() { terminate(); }

void Application::run() {}

void Application::terminate() {}

} // namespace turret