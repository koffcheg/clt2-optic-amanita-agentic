#include "dp1v2/runtime/log_field_sanitizer.hpp"

#include <cctype>
#include <string>

namespace dp1v2 {

std::string sanitize_log_field(std::string_view value, const std::size_t max_length) {
    std::string sanitized;
    const std::size_t reserve_len = value.size() < max_length ? value.size() : max_length;
    sanitized.reserve(reserve_len);

    for (const char ch : value) {
        if (sanitized.size() >= max_length) {
            break;
        }
        if (std::isspace(static_cast<unsigned char>(ch)) || ch == '"' || ch == ';' || ch == '=') {
            sanitized.push_back('_');
        } else {
            sanitized.push_back(ch);
        }
    }

    return sanitized;
}

} // namespace dp1v2
