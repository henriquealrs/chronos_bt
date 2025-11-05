#include "utils/zmq_utils.hpp"

namespace chronos::zmq_utils {

zmq::socket_t make_push(zmq::context_t &ctx, std::string_view endpoint,
                        int hwm, int linger_ms, int sndtimeo_ms) {
    zmq::socket_t socket(ctx, zmq::socket_type::push);
    socket.set(zmq::sockopt::sndhwm, hwm);
    socket.set(zmq::sockopt::linger, linger_ms);
    socket.set(zmq::sockopt::sndtimeo, sndtimeo_ms);
    socket.connect(endpoint.data());
    return socket;
}

zmq::socket_t make_pub(zmq::context_t &ctx, std::string_view endpoint,
                       bool bind_endpoint, int hwm, int linger_ms,
                       int sndtimeo_ms) {
    zmq::socket_t socket(ctx, zmq::socket_type::pub);
    socket.set(zmq::sockopt::sndhwm, hwm);
    socket.set(zmq::sockopt::linger, linger_ms);
    socket.set(zmq::sockopt::sndtimeo, sndtimeo_ms);

    if (bind_endpoint) {
        socket.bind(endpoint.data());
    } else {
        socket.connect(endpoint.data());
    }

    return socket;
}

zmq::socket_t make_sub(zmq::context_t &ctx, std::string_view endpoint,
                       std::string_view topic_filter, bool bind_endpoint,
                       int hwm, int linger_ms, int rcvtimeo_ms) {
    zmq::socket_t socket(ctx, zmq::socket_type::sub);
    socket.set(zmq::sockopt::rcvhwm, hwm);
    socket.set(zmq::sockopt::linger, linger_ms);
    socket.set(zmq::sockopt::rcvtimeo, rcvtimeo_ms);
    socket.set(zmq::sockopt::subscribe, topic_filter.data());

    if (bind_endpoint) {
        socket.bind(endpoint.data());
    } else {
        socket.connect(endpoint.data());
    }

    return socket;
}

zmq::socket_t make_pull(zmq::context_t &ctx, std::string_view endpoint,
                        int hwm, int linger_ms, int rcvtimeo_ms) {
    zmq::socket_t socket(ctx, zmq::socket_type::pull);
    socket.set(zmq::sockopt::rcvhwm, hwm);
    socket.set(zmq::sockopt::linger, linger_ms);
    socket.set(zmq::sockopt::rcvtimeo, rcvtimeo_ms);
    socket.bind(endpoint.data());
    return socket;
}

} // namespace chronos::zmq_utils

