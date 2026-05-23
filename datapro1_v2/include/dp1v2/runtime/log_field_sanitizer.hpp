#pragma once

#include <cstddef>
#include <string>
#include <string_view>

namespace dp1v2 {

std::string sanitize_log_field(std::string_view value, std::size_t max_length = 128);

} // namespace dp1v2
