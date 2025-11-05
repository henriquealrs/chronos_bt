#include <ctime>
#include <gtest/gtest.h>
#include <string>

#include "feed_handler/csv.hpp"

TEST(CsvParse, CsvBreakLines) {
    std::string line = "2023-05-12 04:00:00.020157,116.9,2,7,E-B";
    auto result = csv::ParseLine(line);
    ASSERT_EQ(result.size(), 5);

    result = csv::ParseLine("1, ,1,3,ADSA,\"ASDASD\"");
    ASSERT_EQ(result.size(), 6);

    result = csv::ParseLine("abc");
    ASSERT_EQ(result.size(), 1);

    result = csv::ParseLine("abc,");
    ASSERT_EQ(result.size(), 1);

    result = csv::ParseLine(",abc");
    ASSERT_EQ(result.size(), 2);

    result = csv::ParseLine("");
    ASSERT_EQ(result.size(), 0);

    result = csv::ParseLine(",");
    ASSERT_EQ(result.size(), 1);
}

TEST(CsvParse, CsvParseTypes) {
    std::string line = "2023-05-12 04:00:00.020157,116.9,2,7,E-B";
    auto result = csv::ParseLine(line);
    ASSERT_EQ(result.size(), 5);

    ASSERT_TRUE((csv::to_double(result[1]) - 116.9) < 0.01);
    ASSERT_EQ(csv::to_int(result[2]), 2);
    ASSERT_EQ(csv::to_int(result[3]), 7);

    using namespace std::chrono;
    const sys_days day = 2023y / May / 12;
    const sys_time<microseconds> tp =
        sys_time<microseconds>(day + 4h + 20157us);
    const auto expected_us =
        duration_cast<microseconds>(tp.time_since_epoch()).count();

    ASSERT_EQ(csv::to_epoch_us_fast(result[0]), expected_us);
}
