#include <chrono>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <thread>
#include <zmq.hpp>

#include <gtest/gtest.h>

#include "feed_handler/feed_handler.hpp"
#include "feed_handler/feeder.hpp"
#include "feed_handler/input_parser.hpp"

namespace {

constexpr auto kSampleJsonPath = "../data/sample_data.json";
constexpr auto kNonexistentJsonPath = "../data/does_not_exist.json";

} // namespace

TEST(FeedHandlerConfig, ParsesSampleJson) {
    auto feeds_opt = feeder::ParseInputJson(kSampleJsonPath);
    ASSERT_TRUE(feeds_opt.has_value());

    const auto &feeds = feeds_opt.value();
    ASSERT_EQ(feeds.size(), 2u);

    EXPECT_EQ(feeds[0].ticker, "GOOGL");
    EXPECT_EQ(feeds[0].trades, "../data/googl_trade_2023_05_12.txt");
    EXPECT_EQ(feeds[0].quotes, "../data/goog_quote_2023_05_12.txt");

    EXPECT_EQ(feeds[1].ticker, "MSFT");
    EXPECT_EQ(feeds[1].trades, "../data/msft_trade_2023_05_12.txt");
    EXPECT_EQ(feeds[1].quotes, "../data/msft_quote_2023_05_12.txt");
}

TEST(FeedHandlerConfig, MissingFileReturnsEmptyOptional) {
    auto feeds_opt = feeder::ParseInputJson(kNonexistentJsonPath);
    EXPECT_FALSE(feeds_opt.has_value());
}

TEST(FeedHandlerIntegration, PublishesTradeMessages) {
    zmq::context_t ctx{1};
    zmq::socket_t subscriber(ctx, zmq::socket_type::sub);
    subscriber.set(zmq::sockopt::rcvtimeo, 1000);
    subscriber.set(zmq::sockopt::subscribe, "");
    subscriber.connect("tcp://127.0.0.1:5556");

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::string ticker{"MSFT"};
    auto msft_trades = std::make_unique<Feeder<Trade>>(
        ticker, "../data/msft_trade_2023_05_12.txt");
    ASSERT_TRUE(msft_trades->Init());
    feeder::TradeFeederPtrVec trades;
    trades.emplace_back(std::move(msft_trades));

    feeder::QuoteFeederPtrVec quotes;

    auto publisher = feeder::HandleFeed(std::move(trades), std::move(quotes));
    ASSERT_EQ(publisher->GetTotalThreads(), 1);

    zmq::message_t topic_msg;
    zmq::message_t payload_msg;
    zmq::message_t ts_msg;

    ASSERT_TRUE(subscriber.recv(topic_msg));
    ASSERT_TRUE(subscriber.recv(payload_msg));
    ASSERT_TRUE(subscriber.recv(ts_msg));

    EXPECT_EQ(topic_msg.to_string(), "MSFT|T");
    EXPECT_TRUE(std::abs(std::stod(payload_msg.to_string()) - 310.0) < 1);

    std::uint64_t ts{};
    std::memcpy(&ts, ts_msg.data(), sizeof(ts));
    EXPECT_GT(ts, 0u);
    publisher->Stop();
    publisher->Wait();
}
