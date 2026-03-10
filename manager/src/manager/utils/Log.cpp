//
//  Log.cpp
//  manager
//
//  Created by apple on 19.06.2024.
//

#include "Log.h"

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/attributes/attribute.hpp>
#include <boost/log/attributes/attribute_set.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include <boost/log/attributes/current_process_id.hpp>
#include <boost/log/attributes/current_thread_id.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/core/null_deleter.hpp>
#include <iomanip>
#include <fstream>

namespace cm
{

// BOOST_LOG_ATTRIBUTE_KEYWORD(thread_id, "ThreadID", boost::log::attributes::current_thread_id::value_type)
// BOOST_LOG_ATTRIBUTE_KEYWORD(process_id, "ProcessID", boost::log::attributes::current_process_id::value_type)


static const std::vector<std::string> severity_level_strings
{
    "TRACE",
    "DEBUG",
    "INFO",
    "WARNING",
    "ERROR",
    "CRITICAL"
};

static const std::string severity_level_string_none = "NONE";

const std::string& SeverityLevelToString(boost::log::trivial::severity_level lvl)
{
    if (lvl < severity_level_strings.size())
        return severity_level_strings[lvl];
    return severity_level_string_none;
}

struct SverityDummyTag;

boost::log::formatting_ostream& operator<< (
    boost::log::formatting_ostream& stream,
    boost::log::to_log_manip<boost::log::trivial::severity_level, SverityDummyTag> const& manip
)
{
    if (boost::log::trivial::severity_level lvl = manip.get())
    {
        stream << SeverityLevelToString(lvl);
    }
    else 
    {
        stream << static_cast<int>(lvl);
    }
    return stream;
}

void ConfigureLogs(const char* log_path)
{
    // boost::log::add_file_log(log_pat, 10 * 1024 * 1024, );

    boost::log::add_common_attributes();

    typedef boost::log::sinks::synchronous_sink< boost::log::sinks::text_ostream_backend > text_sink;
    namespace expr = boost::log::expressions;
    boost::shared_ptr< text_sink > file_sink = boost::make_shared< text_sink >();

    if (log_path)
    {
        file_sink->locked_backend()->add_stream(boost::make_shared< std::ofstream >(log_path));
    }
    file_sink->locked_backend()->add_stream(
        boost::shared_ptr<std::ostream>(&std::cout, boost::null_deleter()));

    file_sink->set_formatter
    (
        expr::stream
            // << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%H:%M:%S.%f") << " "
            << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S,%f") << " "
            // << " [" << boost::log::expressions::attr<boost::log::attributes::current_process_id::value_type>("ThreadID")
            // << "] [" << boost::log::expressions::attr<boost::log::attributes::current_thread_id::value_type>("ProcessID") << "] "
            << "[" << expr::attr<boost::log::trivial::severity_level, SverityDummyTag>("Severity") << "] "
            << expr::smessage
    );

    boost::log::core::get()->add_sink(file_sink);

    // boost::shared_ptr<text_sink> console_sink = boost::make_shared<text_sink>();
    // console_sink->locked_backend()->add_stream(
    //     boost::shared_ptr<std::ostream>(&std::cout, boost::null_deleter()));
    // boost::log::core::get()->add_sink(console_sink);

    boost::log::core::get()->set_filter
    (
        boost::log::trivial::severity >= boost::log::trivial::debug
    );
}

const char* currentPidStr()
{
    static std::string gCurrentPidStr;
    static std::atomic<const char*> gCurrentPidStrPtr = nullptr;
    static std::mutex gCurrentPidStrLock;

    if (gCurrentPidStrPtr != nullptr)
        return gCurrentPidStrPtr;

    std::lock_guard<std::mutex> guard(gCurrentPidStrLock);

    if (gCurrentPidStrPtr != nullptr)
        return gCurrentPidStrPtr;

    gCurrentPidStr = std::to_string(::getpid());
    gCurrentPidStrPtr = gCurrentPidStr.c_str();

    return gCurrentPidStrPtr;
}

}
