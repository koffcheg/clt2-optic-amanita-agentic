//
//  Process.cpp
//  manager
//
//  Created by apple on 27.05.2024.
//

#include "Process.h"
#include <thread>
#include <iostream>
#include <mutex>
#include <iomanip>
#include <boost/process.hpp>
#include <sstream>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/process/async_pipe.hpp>
#include "utils/Log.h"
#include "HeartBeatAnalyzer.h"
#include "AsyncStream.h"
#include <istream>


static const int kMaxProcessInterruptionTime = 5000; // milliseconds

namespace cm
{

static inline std::vector<std::string> RunParamsToArguments(const RunParams& params)
{
    std::vector<std::string> arguments;
    
    for (const auto& arg : params.args)
        arguments.push_back(arg);
    
    return arguments;
}

static bool WaitForProcessToFinish(boost::process::child& child, int milliseconds)
{
    auto start = std::chrono::system_clock::now();

    while (child.running())
    {
        auto now = std::chrono::system_clock::now();
        int elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
        if (elapsed > milliseconds)
            return false;

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return true;
}

Process::Process(const RunParams& params)
    : _params(params)
    , _running(true)
{
    
}

Process::~Process()
{
    
}

int Process::run()
{
    if (!_running)
        return 0;
    
    boost::process::ipstream pipe_stream;
    //std::istringstream istream;
    //boost::asio::streambuf pipe_stream;
    boost::asio::io_service ios;
    boost::process::async_pipe async_pipe_stream(ios);
    auto stream = AsyncStream::create();

    std::vector<std::string> arguments = RunParamsToArguments(_params);
    boost::process::child child(
        _params.path,
        boost::process::args(arguments), 
        boost::process::start_dir(_params.workDir),
        boost::process::std_out > stream->nbs().pipe());
    int pid = child.native_handle();
    stream->start();

    {
        std::stringstream ss_info;
        ss_info << child.native_handle() << "-" << _params.name << " at " << _params.group;
        if (_params.destinationId)
        {
            ss_info << " [" << _params.destinationId.value() << "]";
        }
        
        ss_info << " pid:" << pid;

        LOG_INFO << ss_info.str();

        std::stringstream ss_args;

        ss_args << _params.path;

        for (const auto& arg : arguments)
        {
            ss_args << " " << arg;
        }

        LOG_DEBUG << ss_args.str();
    }

    std::unique_ptr<StreamAnalyzer> heartbeat;
    if (_params.heartbeat)
    {
        LOG_INFO << "Starting heartbeat for " << _params.name << "(" << pid << ")";
        heartbeat = std::make_unique<HeartBeatAnalyzer>();
    }
    else
    {
        LOG_INFO << "No heartbeat for " << _params.name;
    }

    while (_running)
    {
        if (!child.running())
            break;

        stream->readOnce();

        std::vector<std::string> lines;
        stream->readAll(lines);

        if (heartbeat)
        {
            lines.push_back(std::string());
            for (auto line : lines)
            {
                auto result = heartbeat->analyze(line);
                if (result == StreamAnalyzer::Result::CancelProcess)
                {
                    LOG_INFO << "Heartbeat failed for " << _params.name << "(" << pid << "), stopping...";
                    cancel();
                }
            }
        }
        
        if (lines.size() == 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // std::string tline;
    // while (std::getline(pipe_stream, tline) && !tline.empty())
    //     std::cout << tline << std::endl;

    if (child.running())
    {
        kill(pid, SIGINT);
        LOG_INFO << "SIGINT sent to " << _params.name << ":" << pid;

        if (WaitForProcessToFinish(child, kMaxProcessInterruptionTime))
        {
            LOG_INFO << "Gracefully closed " << _params.name << ":" << pid;
        }
        else
        {
            LOG_INFO << "Terminated " << _params.name << ":" << pid;
            child.terminate();
        }
    }
    else
    {
        LOG_INFO << "Exited " << _params.name << ":" << pid;
    }

    stream->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // TODO: better solution needed
    
    return child.exit_code();
}

void Process::cancel()
{
    _running = false;
}

}
