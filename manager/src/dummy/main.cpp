//
//  main.cpp
//  dummy
//
//  Created by apple on 27.05.2024.
//

#include <iostream>
#include <thread>
#include <iomanip>
#include <unistd.h>
#include <atomic>
#include <chrono>
#include <functional>
#include "config/CLIOptions.h"

static void RunFor(int duration, int pause, std::function<void()> payload)
{
    auto started = std::chrono::system_clock::now();

    while (true)
    {
        payload();
        auto now = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - started).count();
        if (elapsed > duration)
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(pause));
    }
}

int main(int argc, const char * argv[])
{
    for (int i = 0; i < argc; ++i)
    {
        std::cout << "Arg " + std::to_string(i) << ": " << argv[i] << std::endl;
    }
    
    int pid = getpid();
    srand((unsigned int)time(NULL) | (unsigned int)pid);
    int cnt = 0;

    RunFor(20000, 100, [](){
        std::cout << "HEARTBEAT_TOKEN" << std::endl;
    });

    RunFor(20000, 500, [](){});
    
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100 + rand() % 50));
        cnt++;
        int rval = rand() % 100;
        std::cout << "HEARTBEAT_TOKEN" << std::setw(4) << cnt << " | " << rval << std::endl;
        if (rval == 0)
            break;
    }
    return -1;
}

