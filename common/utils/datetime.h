//
// Created by Emil Hadzhyiev on 24.08.2024.
//

#ifndef TIME_H
#define TIME_H
#include <string>
#include <chrono>


std::string timePointToString(const std::chrono::system_clock::time_point &tp, const std::string &format, bool withMs = true, bool utc = true);
std::chrono::time_point<std::chrono::system_clock> startTime();

#endif //TIME_H
