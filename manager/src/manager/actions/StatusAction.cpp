//
//  StatusAction.cpp
//  manager
//
//  Created by apple on 01.06.2024.
//

#include "StatusAction.h"
#include "Constants.h"
#include "utils/Log.h"

#include <filesystem>
#include <chrono>
#include <thread>

static const size_t kPongTimeoutMilliseconds = 10000;

namespace cm
{

StatusAction::StatusAction(const CLIOptions& options)
{
    LOG_INFO << "Status action initialized";
}

StatusAction::~StatusAction()
{
    
}

int StatusAction::execute(IPMessageQueue& queue)
{
    IPMessage ping;
    ping.type = IPMessageType_Ping;
    ping.message[0] = 0;
    queue.send(ping);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    auto started = std::chrono::system_clock::now();

    for (;;)
    {
        IPMessage pong;
        if (queue.receive(pong) && pong.type == IPMessageType_Pong)
        {
            LOG_INFO << "<PONG_BEGIN>" << pong.message << "<PONG_END>";
            break;
        }

        auto now = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - started).count();
        if (elapsed > kPongTimeoutMilliseconds)
        {
            LOG_INFO << "<PONG_NONE>";
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    return 0;
}

}
