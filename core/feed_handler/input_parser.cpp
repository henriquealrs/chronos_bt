#include "input_parser.hpp"
#include <cstdlib>
#include <exception>
#include <format>
#include <fstream>
#include <iostream>
#include <optional>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

namespace feeder {

std::optional<Feeds> ParseInputJson(const std::string json) {
    std::ifstream f(json);
    if (!f.is_open()) {
        std::cerr << "Failed to open file: " << json << "\n";
        return {};
    }

    std::string json_path = "";
    auto last_slash = json.rfind("/");
    if (last_slash != std::string::npos) {
        json_path = json.substr(0, last_slash + 1);
        std::cout << json_path << "\n";
        // exit(1);
    }

    rapidjson::IStreamWrapper rj_is(f);
    rapidjson::Document doc;
    doc.ParseStream(rj_is);
    if (doc.HasParseError()) {
        std::cerr << "Failed to parse file: " << json
                  << " - Parse error: " << doc.GetParseError() << "\n";
        return {};
    }

    if (!doc.IsArray()) {
        std::cerr << "JSON document should be an array of tickers";
        return {};
    }

    Feeds feeds{};
    for (auto it = doc.Begin(); it != doc.End(); ++it) {
        feeds.emplace_back(
            std::string((*it)["ticker"].GetString()),
            json_path +
                std::string((*it)["feeds"].GetObject()["trades"].GetString()),
            json_path +
                std::string((*it)["feeds"].GetObject()["quotes"].GetString()));
    }

    for (const auto &f : feeds) {
        std::cout << f.ticker << ":\n\t" << f.trades << "\n\t" << f.quotes
                  << "\n\n";
    }

    return {feeds};
}

} // namespace feeder
