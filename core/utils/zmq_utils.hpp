#pragma once

#include <string_view>
#include <zmq.hpp>

namespace chronos::zmq_utils {

zmq::socket_t make_push(zmq::context_t &ctx, std::string_view endpoint,
                        int hwm = 1000, int linger_ms = 0,
                        int sndtimeo_ms = 0);

zmq::socket_t make_pub(zmq::context_t &ctx, std::string_view endpoint,
                       bool bind_endpoint = true, int hwm = 1000,
                       int linger_ms = 0, int sndtimeo_ms = 0);

zmq::socket_t make_sub(zmq::context_t &ctx, std::string_view endpoint,
                       std::string_view topic_filter = "",
                       bool bind_endpoint = false, int hwm = 1000,
                       int linger_ms = 0, int rcvtimeo_ms = 100);

zmq::socket_t make_pull(zmq::context_t &ctx, std::string_view endpoint,
                        int hwm = 1000, int linger_ms = 0,
                        int rcvtimeo_ms = 100);

} // namespace chronos::zmq_utils
