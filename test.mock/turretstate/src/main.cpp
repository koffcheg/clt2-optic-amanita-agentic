#include "application.hpp"
#include "args.hpp"
#include <csignal>
#include <cstddef>
#include <iostream>
#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>
#include <memory>

std::unique_ptr<turret::Application> application;

static auto logger = log4cxx::Logger::getLogger("turretstate");

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

bool configureLogger(const std::string &loggerConfig) {
  auto configuration_status =
      log4cxx::xml::DOMConfigurator::configureAndWatch(loggerConfig);
  if (configuration_status ==
      log4cxx::spi::ConfigurationStatus::NotConfigured) {
    std::cerr << "FATAL: "
              << "unable to configure logging subsystem from cfg.file: "
              << loggerConfig << std::endl;
    return false;
  }
  return true;
}

int main(int argc, const char *argv[]) {
  int errorCode = -1; // todo: define error codes
  try {
    // Install a signal handler
    std::signal(SIGINT, signalHandler);

    turret::Args args;

    if (!args.parse(argc, argv, errorCode)) {
      return errorCode;
    }

    if (!configureLogger(args.loggerConfig)) {
      return -1;
    };

    // replace a temporary logger with a permanent one
    logger = log4cxx::Logger::getLogger("turretstate");

    // Create an application object
    application =
        std::make_unique<turret::Application>(args.infoHost, args.infoPort);

    // Run the application
    errorCode = application->run();
    if (errorCode) {
      LOG4CXX_ERROR(logger,
                    "turretstate terminated abnormally. err=" << errorCode);
    } else {
      LOG4CXX_INFO(logger, "turretstate exited");
    }
  } catch (const std::exception &e) {
    // todo: application hangs up if exception is caught (presumambly connected
    // with log4cxx)
    LOG4CXX_ERROR(logger, "std::exception: " << e.what());
    errorCode = -3;
  } catch (...) {
    LOG4CXX_ERROR(logger, "Unknown exception");
    errorCode = -4;
  }

  return errorCode;
}