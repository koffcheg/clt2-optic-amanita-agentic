//
//  IPMessage.h
//  manager
//
//  Created by apple on 31.05.2024.
//

#pragma once

#include <cstdint>
#include <cstring>

namespace cm
{
    static const size_t kIPMessageBufferSize = 128;
    static const uint32_t IPMessageType_Unknown = 0;
    static const uint32_t IPMessageType_Stop = 1;
    static const uint32_t IPMessageType_Ping = 2;
    static const uint32_t IPMessageType_Pong = 3;

    struct IPMessage
    {
        IPMessage(uint32_t a_type = IPMessageType_Unknown)
            : type(a_type)
        {}
        
        uint32_t type;
        char message[kIPMessageBufferSize];
    };
}
