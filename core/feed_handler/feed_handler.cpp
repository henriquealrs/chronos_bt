#include "feed_handler.hpp"
#include "feeder.hpp"
#include "publisher.hpp"
#include "utils/thread_utils.hpp"

#include <chrono>
#include <exception>
#include <iostream>
#include <memory>
#include <stop_token>
#include <string>
#include <thread>
#include <utility>
#include <vector>
#include <zmq.hpp>

namespace feeder {

namespace {

constexpr std::string OUT_PUB = "tcp://*:5556";

} // namespace

[[nodiscard]] PublisherPtr HandleFeed(TradeFeederPtrVec &&trades_feed,
                                      QuoteFeederPtrVec &&quotes_feed) {

    try {
        std::cout << "Making publisher\n";
        auto publisher =
            std::make_unique<Publisher>(OUT_PUB, trades_feed, quotes_feed);
        try {
            publisher->Start();
            std::this_thread::yield();
            std::this_thread::yield();
            return publisher;
        } catch (const std::exception &e) {
            std::cout << "Exception starting publishe\n";
            throw e;
        }
    } catch (...) {
        std::cout << "exception making pub\n";
    }
    return nullptr;
}

} // namespace feeder
