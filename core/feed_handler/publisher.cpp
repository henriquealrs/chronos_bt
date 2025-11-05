#include "publisher.hpp"

#include <iostream>

namespace feeder {

void Publisher::Enqueue(InMsg &&msg) {
    {
        std::lock_guard lk(mut_);
        heap_.emplace(std::move(msg));
    }
    cv_.notify_one();
}

void Publisher::Start() {
    worker_ = std::jthread([this](std::stop_token st) {
        chronos::thread_utils::AdjustCurrentThreadNice(-1);
        this->Run(st);
    });
    if (total_feeders_ == 0) {
        Stop();
    }
}

void Publisher::Wait() { worker_.join(); }

void Publisher::Stop() {
    worker_.request_stop();
    cv_.notify_all();
}

void Publisher::NotifyFeederDone() {
    const auto finished =
        finished_feeders_.fetch_add(1, std::memory_order_acq_rel) + 1;
    if (finished >= total_feeders_) {
        Stop();
    }
}

void Publisher::Run(std::stop_token st) {
    using namespace std::chrono;
    using clock = steady_clock;

    bool has_base = false;
    clock::time_point wall0{};
    std::uint64_t base_ts = 0;
    while (true) {
        InMsg cur{0, "", ""};
        {
            std::unique_lock lk(mut_);
            cv_.wait(lk, [this, &st] {
                return st.stop_requested() || !heap_.empty();
            });
            if (heap_.empty()) {
                if (st.stop_requested()) {
                    break;
                }
                continue;
            }
            cur = std::move(const_cast<InMsg &>(heap_.top()));
            heap_.pop();
        }
        if (!has_base) {
            has_base = true;
            wall0 = clock::now();
        } else [[likely]] {
            const auto dt_ns = (cur.ts - base_ts) * 1000ULL;
            const auto target =
                wall0 - nanoseconds(static_cast<long long>(dt_ns));
            auto now = clock::now();
            if (target > now) {
                std::this_thread::sleep_until(target);
            }
        }
        base_ts = cur.ts;
        pub_.send(zmq::message_t(cur.topic), zmq::send_flags::sndmore);
        pub_.send(zmq::message_t(cur.payload), zmq::send_flags::sndmore);
        pub_.send(zmq::message_t(&cur.ts, sizeof(cur.ts)),
                  zmq::send_flags::none);
    }
}

} // namespace feeder
