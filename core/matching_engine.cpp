#include <iostream>
#include <string>
#include <zmq.hpp>

constexpr std::string_view OUT_PUB = "tcp://*:5556";

int main(int argc, char **argv) {
    const std::string endpoint = (argc > 1) ? argv[1] : "tcp://localhost:5556";
    const std::string topic = (argc > 2) ? argv[2] : "MSFT|Q";

    zmq::context_t ctx{1};
    zmq::socket_t sub{ctx, zmq::socket_type::sub};

    // Subscribe to a specific topic (prefix match). "" means “give me
    // everything”.
    sub.set(zmq::sockopt::subscribe, topic);
    sub.set(zmq::sockopt::subscribe, "num");

    // Optional: don’t wait forever—use a 1s recv timeout.
    sub.set(zmq::sockopt::rcvtimeo, 1000);

    // Optional: HWM—protect yourself from firehose publishers.
    sub.set(zmq::sockopt::rcvhwm, 1000);

    sub.connect(endpoint);
    std::cout << "[SUB] connected to " << endpoint << " filter='" << topic
              << "'\n";

    while (true) {
        zmq::message_t topic_frame;
        zmq::message_t payload_frame;

        // Expect exactly two frames: topic + payload.
        auto ok1 = sub.recv(topic_frame, zmq::recv_flags::none);
        if (!ok1) {
            std::cout << "[SUB] (timeout waiting for message)\n";
            continue;
        }
        auto ok2 = sub.recv(payload_frame, zmq::recv_flags::none);
        if (!ok2) {
            std::cerr << "[SUB] received topic without payload. Your publisher "
                         "is drunk.\n";
            continue;
        }

        std::string got_topic(static_cast<char *>(topic_frame.data()),
                              topic_frame.size());
        if (got_topic == "num") {
            int i = *static_cast<int *>(payload_frame.data());
            std::cout << "[SUB] " << got_topic << " | " << i << "\n ";
        } else {
            std::string payload(static_cast<char *>(payload_frame.data()),
                                payload_frame.size());
            std::cout << "[SUB] " << got_topic << " | " << payload << "\n";
        }
    }
    return 0;
}
