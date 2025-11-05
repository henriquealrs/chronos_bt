#include <charconv>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

namespace csv {

static std::vector<std::string_view> ParseLine(std::string_view line,
                                               char delimiter = ',') {
    std::vector<std::string_view> ret;
    std::string::size_type p = 0;
    ret.reserve(6);
    while (1) {
        auto q = line.find(delimiter, p);
        if (q == std::string_view::npos) {
            if (p != (line.size())) {
                ret.emplace_back(line.substr(p));
            }
            break;
        }
        ret.emplace_back(line.substr(p, q - p));
        p = (q + 1);
    }
    return ret;
}

// fast parse helpers
inline int to_int(std::string_view sv) {
    int v{};
    auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), v);
    if (ec != std::errc())
        throw std::runtime_error("bad int");
    return v;
}

inline double to_double(std::string_view sv) {
    // C++17 has no from_chars for double everywhere; if your stdlib supports
    // it, use it.
    return std::stod(
        std::string(sv)); // replace with from_chars<double> if available.
}
inline uint64_t to_u64(std::string_view sv) {
    uint64_t v{};
    auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), v);
    if (ec != std::errc())
        throw std::runtime_error("bad u64");
    return v;
}

inline uint64_t to_epoch_us_fast(std::string_view ts) {
    // Fixed positions; no heap use.
    int y, m, d, H, M, S, u = 0;
    auto num = [&](size_t p, size_t n, int &out) {
        auto [ptr, ec] = std::from_chars(ts.data() + p, ts.data() + p + n, out);
        if (ec != std::errc())
            throw std::runtime_error("bad ts field");
    };
    num(0, 4, y);
    num(5, 2, m);
    num(8, 2, d);
    num(11, 2, H);
    num(14, 2, M);
    num(17, 2, S);
    if (ts.size() > 20 && ts[19] == '.')
        num(20, 6, u);

    std::tm tm{};
    tm.tm_year = y - 1900;
    tm.tm_mon = m - 1;
    tm.tm_mday = d;
    tm.tm_hour = H;
    tm.tm_min = M;
    tm.tm_sec = S;
    std::time_t s = timegm(&tm);
    return static_cast<uint64_t>(s) * 1'000'000ULL + u;
}

} // namespace csv
