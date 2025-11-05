#pragma once

#include <numeric>
#include <optional>
#include <string>
#include <vector>

#include "feed_handler.hpp"

namespace feeder {

[[nodiscard]] std::optional<Feeds> ParseInputJson(const std::string json);

}
