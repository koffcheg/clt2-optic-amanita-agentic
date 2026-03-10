//
//  Log.h
//  manager
//
//  Created by apple on 19.06.2024.
//

#pragma once

#include <boost/log/trivial.hpp>
#include <boost/log/sources/severity_logger.hpp>

#define	LOG_TRACE \
	BOOST_LOG_TRIVIAL(trace) << currentPidStr() << __FILE__ << ":" << __LINE__ << " " << __PRETTY_FUNCTION__ << ": "

#define	LOG_DEBUG \
	BOOST_LOG_TRIVIAL(debug) << currentPidStr() << __FILE__ << ":" << __LINE__ << " " << __PRETTY_FUNCTION__ << ": "

#define	LOG_INFO \
	BOOST_LOG_TRIVIAL(info) << currentPidStr() << __FILE__ << ":" << __LINE__ << " " << __PRETTY_FUNCTION__ << ": "

#define	LOG_WARNING \
	BOOST_LOG_TRIVIAL(warning) << currentPidStr() << __FILE__ << ":" << __LINE__ << " " << __PRETTY_FUNCTION__ << ": "

#define	LOG_ERROR \
	BOOST_LOG_TRIVIAL(error) << currentPidStr() << __FILE__ << ":" << __LINE__ << " " << __PRETTY_FUNCTION__ << ": "

#define	LOG_FATAL \
	BOOST_LOG_TRIVIAL(fatal) << currentPidStr() << __FILE__ << ":" << __LINE__ << " " << __PRETTY_FUNCTION__ << ": "

namespace cm
{
    void ConfigureLogs(const char* log_path);
    const char* currentPidStr();

}
