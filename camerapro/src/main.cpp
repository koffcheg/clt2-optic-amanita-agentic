#include "application.hpp"
#include "args.hpp"
#include "cp_config.h"
#include <csignal>
#include <cstddef>
#include <iostream>
#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>
#include <memory>

std::unique_ptr<cam_pro::Application> application;

static auto logger = log4cxx::Logger::getLogger("cam");

void signalHandler(int signal) {
  if (signal == SIGINT) {
    if (application) {
      if (!application->terminated()) {
        LOG4CXX_INFO(logger, "SIGINT caught, exiting...");
        application->terminate();
      } else {
        LOG4CXX_WARN(logger, "Another SIGINT caught. Force exit");
        exit(3);
      }
    } else {
      LOG4CXX_WARN(logger, "SIGINT caught. No application. Exit.");
      exit(4);
    }
  }
}

bool configureLogger(const cam_pro::Args &args) {
  auto configuration_status =
      log4cxx::xml::DOMConfigurator::configureAndWatch(args.loggerConfig);
  if (configuration_status ==
      log4cxx::spi::ConfigurationStatus::NotConfigured) {
    std::cerr << "FATAL: "
              << "unable to configure logging subsystem from cfg.file: "
              << args.loggerConfig << std::endl;
    return false;
  }
  return true;
}

int main(int argc, const char *argv[]) {
  int errorCode = -1; // todo: define error codes
  try {
    // Install a signal handler
    std::signal(SIGINT, signalHandler);

    cam_pro::Args args;

    if (!args.parse(argc, argv, errorCode)) {
      return errorCode;
    }

    if (!configureLogger(args)) {
      return -1;
    };

    // replace a temporary logger with a permanent one
    logger = log4cxx::Logger::getLogger("cam" + std::to_string(args.cameraID));

    const cam_pro::Config config(args.mainConfig, args.cameraID);

    // Create an application object
    application = std::make_unique<cam_pro::Application>(config);

    // Run the application
    errorCode = application->run();
    if (errorCode) {
      LOG4CXX_ERROR(logger,
                    "camerapro terminated abnormally. err=" << errorCode);
    } else {
      LOG4CXX_INFO(logger, "camerapro exited");
    }
  } catch (const std::exception &e) {
    LOG4CXX_ERROR(logger, "std::exception: " << e.what());
    errorCode = -3;
  } catch (...) {
    LOG4CXX_ERROR(logger, "Unknown exception");
    errorCode = -4;
  }

  return errorCode;
}