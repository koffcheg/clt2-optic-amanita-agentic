#include "application.hpp"
#include "args.hpp"
#include "config.hpp"
#include "utils/decimalSeparator.hpp"
#include "window.hpp"
#include <QApplication>
#include <QSurfaceFormat>
#include <csignal>
#include <iostream>
#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>

std::unique_ptr<turret::Application> application;

static auto logger = log4cxx::Logger::getLogger("plotter");

void signalHandler(int signal) {
  if (signal == SIGINT) {
    if (application) {
      /*todo:
      if (!application->terminated()) {
        LOG4CXX_INFO(logger, "SIGINT caught, exiting...");
        window->terminate();
      } else {
        LOG4CXX_WARN(logger, "Another SIGINT caught. Force exit");
        exit(3);
      }*/
    } else {
      LOG4CXX_WARN(logger, "SIGINT caught. No application. Exit.");
      exit(4);
    }
  }
}

bool configureLogger(const turret::Args &args) {
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

int main(int argc, char *argv[]) {
  int errorCode = -1; // todo: define error codes
  try {
    setDecimalSeparator('.');

    // Install a signal handler
    std::signal(SIGINT, signalHandler);

    turret::Args args;

    if (!args.parse(argc, const_cast<const char **>(argv), errorCode)) {
      return errorCode;
    }

    if (!configureLogger(args)) {
      return -1;
    };

    // replace a temporary logger with a permanent one
    logger = log4cxx::Logger::getLogger("plotter");

    const turret::Config config(args.mainConfig);

    // Create an application object
    application = std::make_unique<turret::Application>(config);

    application->setFilename(args.fileToOpen);

    // Run the application
    application->run();
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":icons/plotter.png"));
    Window window(*application.get());
    window.show();

    setDecimalSeparator('.');

    errorCode = app.exec();
    application->terminate();
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
