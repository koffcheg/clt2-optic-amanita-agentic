
#pragma once

#include <log4cxx/logger.h>

#define LOG_TRACE(...) LOG4CXX_TRACE(logger, __VA_ARGS__)
#define LOG_DEBUG(...) LOG4CXX_DEBUG(logger, __VA_ARGS__)
#define LOG_INFO(...) LOG4CXX_INFO(logger, __VA_ARGS__)
#define LOG_WARN(...) LOG4CXX_WARN(logger, __VA_ARGS__)
#define LOG_ERROR(...) LOG4CXX_ERROR(logger, __VA_ARGS__)
#define LOG_FATAL(...) LOG4CXX_FATAL(logger, __VA_ARGS__)
