#pragma once

#include "feeder.hpp"

#include "publisher.hpp"

#include <memory>
#include <string>
#include <vector>

namespace feeder {

struct Feed {
    std::string ticker;
    std::string trades;
    std::string quotes;
    Feed() = delete;
    Feed(const std::string ticker_, const std::string trades_,
         const std::string quotes_)
        : ticker{ticker_}, trades{trades_}, quotes{quotes_} {}
};

using TradeFeederPtrVec = std::vector<std::unique_ptr<Feeder<Trade>>>;
using QuoteFeederPtrVec = std::vector<std::unique_ptr<Feeder<Quote>>>;

using Feeds = std::vector<Feed>;

[[nodiscard]] PublisherPtr HandleFeed(TradeFeederPtrVec &&trades,
                                      QuoteFeederPtrVec &&quotes);

} // namespace feeder
