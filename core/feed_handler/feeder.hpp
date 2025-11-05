#pragma once

#include <algorithm>
#include <cstdint>
#include <format>
#include <fstream>
#include <iterator>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "csv.hpp"

struct Trade {
    std::string sym;
    double px;
    int sz;
    std::uint64_t ts;
    Trade(std::string sym_, double px_, int sz_, std::uint64_t ts_)
        : sym(std::move(sym_)), px(px_), sz(sz_), ts(ts_) {}

    Trade() = delete;
    // Trade(const Trade &other) = delete;
    // Trade &operator=(const Trade &other) = delete;

    Trade(Trade &&o) noexcept = default;
    Trade &operator=(Trade &&o) noexcept = default;

    ~Trade() = default;
};

struct Quote {
    std::string sym;
    double bid, ask;
    int bidSz, askSz;
    std::uint64_t ts;

    Quote(std::string sym_, double bid_, double ask_, int bitSz_, int askSz_,
          std::uint64_t ts_)
        : sym(sym_), bid(bid_), ask(ask_), bidSz(bitSz_), askSz(askSz_),
          ts(ts_) {}

    Quote() = delete;
    ~Quote() = default;

    // Quote(const Quote &o) = delete;
    // Quote &operator=(const Quote &o) = delete;

    Quote(Quote &&o) noexcept = default;
    Quote &operator=(Quote &&o) noexcept = default;
};

template <class T> struct FeedTraits;

template <> struct FeedTraits<Trade> {
    std::string sym;
    FeedTraits() = delete;
    FeedTraits(std::string_view sym_) : sym{sym_} {}

    Trade Parse(std::string_view line) {
        auto values = csv::ParseLine(line);

        Trade trade(std::string(sym), csv::to_double(values[1]),
                    csv::to_int(values[2]), csv::to_epoch_us_fast(values[0]));

        return trade;
    }

    std::pair<std::string, std::string> Serialize(const Trade &q) {
        std::string topic = q.sym + "|T";
        std::string data = std::format("{},{}", q.px, q.sz);
        return std::make_pair(topic, data);
    }
};

template <> struct FeedTraits<Quote> {
    std::string sym;
    FeedTraits() = delete;
    FeedTraits(std::string_view sym_) : sym{sym_} {}

    Quote Parse(std::string_view line) {
        auto values = csv::ParseLine(line);

        Quote quote(std::string(sym), csv::to_double(values[1]),
                    csv::to_double(values[4]), csv::to_int(values[2]),
                    csv::to_int(values[5]), csv::to_epoch_us_fast(values[0]));
        return quote;
    }

    std::pair<std::string, std::string> Serialize(const Quote &q) {
        std::string topic = q.sym + "|Q";
        std::string data =
            std::format("{},{},{},{}", q.bid, q.bidSz, q.ask, q.askSz);
        return std::make_pair(topic, data);
    }
};

struct SerData {
    std::string topic;
    std::string payload;
    uint64_t ts;
};

template <class T, class Traits = FeedTraits<T>> class Feeder {
    std::string ticker_;
    std::ifstream fd_;
    Traits t_;

  public:
    Feeder() = delete;
    Feeder(const Feeder &other) = delete;
    Feeder &operator=(const Feeder &other) = delete;

    Feeder(std::string ticker, const std::string &feed_file)
        : ticker_{ticker}, fd_(feed_file), t_(ticker) {}

    bool Init() { return fd_.is_open(); }
    [[nodiscard]] std::optional<T> ReadLine() {
        std::optional<T> ret;
        std::string line;
        if (!std::getline(fd_, line)) {
            return ret;
        }

        return std::move(ret.emplace(t_.Parse(line)));
    }

    SerData Serialize(const T &data) {
        const auto [topic, payload] = t_.Serialize(data);
        return {topic, payload, data.ts};
    }
};
