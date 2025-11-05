#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

#include <rapidjson/rapidjson.h>
#include <zmq.hpp>

#include "feed_handler.hpp"
#include "feeder.hpp"
#include "input_parser.hpp"

namespace chronos {

struct FeedHandlerConfig {
    std::filesystem::path input_path;
};

namespace {

void PrintUsage(const std::string_view program_name) {
    std::cerr << "Usage: " << program_name
              << " --input <file.json>\n"
                 "  -i, --input   Path to input JSON file\n"
                 "  -h, --help    Show this message\n";
}

FeedHandlerConfig ParseArgs(const int argc, char *argv[]) {
    if (argc <= 1) {
        PrintUsage(argv[0]);
        throw std::invalid_argument("Missing arguments");
    }

    FeedHandlerConfig config{};
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg{argv[i]};
        if (arg == "-h" || arg == "--help") {
            PrintUsage(argv[0]);
            std::exit(EXIT_SUCCESS);
        } else if (arg == "-i" || arg == "--input") {
            if (i + 1 >= argc) {
                throw std::invalid_argument("Expected path after --input");
            }
            config.input_path = argv[++i];
        } else {
            throw std::invalid_argument("Unknown argument: " +
                                        std::string(arg));
        }
    }

    if (config.input_path.empty()) {
        throw std::invalid_argument("Input JSON file is required");
    }
    if (!std::filesystem::exists(config.input_path)) {
        throw std::invalid_argument("Input file does not exist: " +
                                    config.input_path.string());
    }
    if (config.input_path.extension() != ".json") {
        throw std::invalid_argument("Input file must use .json extension");
    }

    return config;
}

void PopulateQnTFeeders(const feeder::Feeds &inputs,
                        feeder::TradeFeederPtrVec &trades,
                        feeder::QuoteFeederPtrVec &quotes) {
    for (const auto &i : inputs) {
        trades.emplace_back(
            std::make_unique<Feeder<Trade>>(i.ticker, i.trades));
        if (!trades.back()->Init()) {
            std::cout << "Error opening file: " << i.trades << "\n";
            exit(1);
        }
        quotes.emplace_back(
            std::make_unique<Feeder<Quote>>(i.ticker, i.quotes));
    }
}

} // namespace

int FeedHandlerMain(const int argc, char *argv[]) {
    const auto config = ParseArgs(argc, argv);
    std::cout << "Using input JSON: " << config.input_path << '\n';
    auto inputs = feeder::ParseInputJson(config.input_path);
    if (!inputs || inputs.value().empty()) {
        return 1;
    }
    feeder::TradeFeederPtrVec trade_feeds;
    feeder::QuoteFeederPtrVec quote_feeds;
    PopulateQnTFeeders(*inputs, trade_feeds, quote_feeds);
    // TODO(chronos): Load and process the JSON feed here.
    try {
        auto pub =
            feeder::HandleFeed(std::move(trade_feeds), std::move(quote_feeds));
        pub->Wait();
    } catch (const std::exception &e) {
        std::cout << "Exception handling feed: " << e.what() << '\n';
    }
    return EXIT_SUCCESS;
}

} // namespace chronos

int main(int argc, char *argv[]) {
    try {
        return chronos::FeedHandlerMain(argc, argv);
    } catch (const std::exception &ex) {
        std::cerr << "Feed handler error: " << ex.what() << '\n';
        return EXIT_FAILURE;
    }
}
