//
//  Payload.h
//  manager
//
//  Created by apple on 27.05.2024.
//

#pragma once

namespace cm
{
    class Payload
    {
    public:
        virtual ~Payload() {}
        
        virtual int run() = 0;
    };

}
