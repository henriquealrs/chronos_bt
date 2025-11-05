#include "feed_handler/feeder.hpp"
#include <gtest/gtest.h>

TEST(FeederTest, QuotesParsing) {
    std::string sym = "MSFT";
    Feeder<Quote> quote_feed(sym, "../data/msft_quote_2023_05_12.txt");
    ASSERT_TRUE(quote_feed.Init());

    auto opt_line = quote_feed.ReadLine();
    ASSERT_TRUE(opt_line.has_value());
    auto line = std::move(opt_line.value());

    EXPECT_EQ(line.sym, sym);
    // "2023-05-12 04:00:00.004,309.77,200,1,310.41,200,1"
    using namespace std::chrono;
    const sys_days day = 2023y / May / 12;
    const sys_time<microseconds> tp = sys_time<microseconds>(day + 4h + 0004us);
    const auto expected_us =
        duration_cast<microseconds>(tp.time_since_epoch()).count();
    EXPECT_EQ(expected_us, line.ts);
    EXPECT_EQ(line.bidSz, 200);
    EXPECT_EQ(line.bid, 309.77);

    EXPECT_EQ(line.ask, 310.41);
    EXPECT_EQ(line.askSz, 200);
}

TEST(FeederTest, TradesParsing) {
    std::string sym = "MSFT";
    Feeder<Trade> trade_feed(sym, "../data/msft_trade_2023_05_12.txt");
    ASSERT_TRUE(trade_feed.Init());

    auto opt_line = trade_feed.ReadLine();
    ASSERT_TRUE(opt_line.has_value());
    auto line = std::move(opt_line.value());

    EXPECT_EQ(line.sym, sym);
    // 2023-05-12 04:00:00.027477,309.9,3,7,E-B
    using namespace std::chrono;
    const sys_days day = 2023y / May / 12;
    const sys_time<microseconds> tp =
        sys_time<microseconds>(day + 4h + 27477us);
    const auto expected_us =
        duration_cast<microseconds>(tp.time_since_epoch()).count();
    EXPECT_EQ(expected_us, line.ts);
    EXPECT_DOUBLE_EQ(line.px, 309.9);
    EXPECT_EQ(line.sz, 3);
}
