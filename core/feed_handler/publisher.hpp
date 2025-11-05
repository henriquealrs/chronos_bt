#pragma once

#include "utils/thread_utils.hpp"
#include "utils/zmq_utils.hpp"

#include "feeder.hpp"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <stop_token>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>
#include <zmq.hpp>

namespace feeder {

struct InMsg {
    std::uint64_t ts;
    std::string topic;
    std::string payload;
    InMsg() = delete;
    InMsg(const InMsg &other) = delete;
    InMsg &operator=(const InMsg &other) = delete;
    InMsg(std::uint64_t ts_, std::string topic_, std::string payload_)
        : ts{ts_}, topic{std::move(topic_)}, payload(std::move(payload_)) {}
    InMsg(InMsg &&other) noexcept = default;
    InMsg &operator=(InMsg &&o) noexcept = default;

    bool operator>(const InMsg &o) const noexcept { return ts > o.ts; }
};

class Publisher {
  public:
    Publisher() = delete;
    Publisher(const Publisher &other) = delete;
    Publisher &operator=(const Publisher &other) = delete;

    template <typename... FeedVectPtr>
    Publisher(std::string endpoint, FeedVectPtr &&...feed)
        : pub_(chronos::zmq_utils::make_pub(zmq_ctx_, endpoint)),
          total_feeders_((feed.size() + ...)) {
        (SpawnFeeds(feed), ...);
    }

    template <class T>
    void FeedLoop(std::unique_ptr<Feeder<T>> feed, std::stop_token stoken) {
        chronos::thread_utils::AdjustCurrentThreadNice(+1);
        while (!stoken.stop_requested()) {
            if (auto data_opt = feed->ReadLine(); data_opt.has_value()) {
                auto [topic, value, ts] = feed->Serialize(*data_opt);
                InMsg msg{ts, topic, value};
                Enqueue(std::move(msg));
            } else {
                break;
            }
        }
        NotifyFeederDone();
    }

    template <class FeedPtrVec> void SpawnFeeds(FeedPtrVec &&feeds) {
        threads_.reserve(threads_.size() + feeds.size());
        for (auto &feed : feeds) {
            threads_.emplace_back(
                [pf = std::move(feed), this](std::stop_token st) mutable {
                    FeedLoop(std::move(pf), st);
                });
        }
    }

    void Enqueue(InMsg &&msg);
    void Start();
    void Wait();
    void Stop();
    void NotifyFeederDone();
    [[nodiscard]] constexpr std::size_t GetTotalThreads() const noexcept {
        if (std::is_constant_evaluated()) {
            std::cout << " Compile time\n";
        } else {
            std::cout << "Not compile time\n";
        }
        return total_feeders_;
    }

  private:
    using MessagesHeap =
        std::priority_queue<InMsg, std::vector<InMsg>, std::greater<InMsg>>;

    void Run(std::stop_token st);

    std::mutex mut_;
    std::condition_variable cv_;
    MessagesHeap heap_;
    zmq::context_t zmq_ctx_;
    zmq::socket_t pub_;
    std::jthread worker_;
    std::size_t total_feeders_;
    std::atomic<std::size_t> finished_feeders_{0};
    std::vector<std::jthread> threads_;
};

using PublisherPtr = std::unique_ptr<Publisher>;

} // namespace feeder
