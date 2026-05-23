#include <gtest/gtest.h>

#include <string>

#include "dp1v2/runtime/log_field_sanitizer.hpp"

TEST(LogFieldSanitizerTest, ReplacesWhitespaceCharacters)
{
    EXPECT_EQ(dp1v2::sanitize_log_field("alpha beta"), "alpha_beta");
    EXPECT_EQ(dp1v2::sanitize_log_field("a\tb\nc\rd"), "a_b_c_d");
}

TEST(LogFieldSanitizerTest, ReplacesReservedSymbols)
{
    EXPECT_EQ(dp1v2::sanitize_log_field("a=b;c\"d"), "a_b_c_d");
}

TEST(LogFieldSanitizerTest, TruncatesLongValues)
{
    std::string input(200, 'a');
    const std::string output = dp1v2::sanitize_log_field(input);

    EXPECT_EQ(output.size(), 128U);
    EXPECT_EQ(output, std::string(128, 'a'));
}
