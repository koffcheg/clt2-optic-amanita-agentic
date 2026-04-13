#include <iostream>

#include "dp1v2/frame_packet.hpp"

int main()
{
    dp1v2::FramePacket packet{};
    std::cout << "datapro1_v2 scaffold started, frame_id=" << packet.frame_id << std::endl;
    return 0;
}
