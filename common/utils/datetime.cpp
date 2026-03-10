#include "datetime.h"

#include <iomanip>
#include <sstream>

std::string timePointToString(const std::chrono::system_clock::time_point &tp, const std::string &format,
                              bool withMs, bool utc) {
    const std::chrono::system_clock::time_point::duration tt = tp.time_since_epoch();
    const time_t durS = std::chrono::duration_cast<std::chrono::seconds>(tt).count();
    std::ostringstream ss;
    if (const std::tm *tm = (utc ? std::gmtime(&durS) : std::localtime(&durS))) {
        ss << std::put_time(tm, format.c_str());
        if (withMs) {
            const long long durMs = std::chrono::duration_cast<std::chrono::milliseconds>(tt).count();
            ss << std::setw(3) << std::setfill('0') << int(durMs - durS * 1000);
        }
    }
    else {
        ss << "<FORMAT ERROR>";
    }
    return ss.str();
}

std::chrono::time_point<std::chrono::system_clock> startTime(){
    auto now = std::chrono::system_clock::now();
    // Отримуємо час у форматі години, хвилини, секунди
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm* now_tm = std::localtime(&now_c);

    // Встановлюємо нульову годину
    now_tm->tm_hour = 0;
    now_tm->tm_min = 0;
    now_tm->tm_sec = 0;
    // Конвертуємо назад в time_point
    auto zeroHourPoint = std::chrono::system_clock::from_time_t(std::mktime(now_tm));
    return zeroHourPoint;
}
