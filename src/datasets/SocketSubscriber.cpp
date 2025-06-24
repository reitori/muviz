#include "include/SocketSubscriber.h"
#include "logging.h"

namespace {
    auto logger = logging::make_log("SocketSubscriber");
    bool SocketSubscriberRegistered =
      StdDict::registerDataLoader("SocketSubscriber",
                                  [](){ return std::unique_ptr<DataLoader>(new SocketSubscriber()); });
}

SocketSubscriber::SocketSubscriber()
  : context(1),
    total_events(0),
    packet_counter(0),
    total_hits(0),
    current_retry(0),
    max_retry_delay(60000),
    max_connection_retries(10),
    run_thread(false),
    is_connected(false)
{}

SocketSubscriber::~SocketSubscriber() {
    if (subscriber) subscriber->close();
}

void SocketSubscriber::init() {
    total_events = 0;
    total_hits = 0;
    current_retry = 0;
    run_thread = false;
    is_connected = false;
    connectToServer(true);
}

void SocketSubscriber::configure(const json &config) {
    if (config.contains("name")) {
        name = config["name"];
    } else {
        logger->error("No name provided! Please add name to all sources. Assuming name = None");
        throw std::invalid_argument("No name provided!");
    }

    if (config.contains("server_ip")) {
        server_ip = config["server_ip"].get<std::string>();
    } else {
        logger->warn("\"server_ip\" not provided. Assuming local host.");
        server_ip = "127.0.0.1";
    }

    if (config.contains("port")) {
        port = config["port"].get<std::string>();
    } else {
        logger->error("No port is provided for {}", name);
        throw std::invalid_argument("No port provided!");
    }

    if (config.contains("max_connection_retries"))
        max_connection_retries = config["max_connection_retries"];
    else
        max_connection_retries = 10;

    if (config.contains("max_retry_delay"))
        max_retry_delay = config["max_retry_delay"];
    else
        max_retry_delay = 60000;
}

void SocketSubscriber::run() {
    run_thread = true;
    thread_ptr = std::make_unique<std::thread>(&SocketSubscriber::process, this);
}

void SocketSubscriber::join() {
    run_thread = false;
    thread_ptr->join();
    logger->info("[{}]: Processed {} events, with {} hits, in {} batches",
                 name, total_events, total_hits, packet_counter);
}

void SocketSubscriber::process() {
    uint32_t batch_n = 0;
    curEvents = std::make_unique<EventData>();

    auto last = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point now;

    while (run_thread) {
        if (is_connected) {
            std::vector<uint8_t> rawbytes;
            if (!getPacket(rawbytes)) {
                logger->debug("Failed to get {} packet", packet_counter);
                continue;
            }
            processPacket(rawbytes);

            now = std::chrono::steady_clock::now();
            if (curEvents->size() > 0) {
                float diff = std::chrono::duration_cast<std::chrono::microseconds>(now - last).count() / 1e6f;
                logger->debug(
                    "[{}] Batch {}: {} events in {} seconds = {} ev/s TotalEvents: {}",
                    name, batch_n, curEvents->size(), diff, curEvents->size() / diff, total_events
                );
                output->pushData(std::move(curEvents));
                curEvents = std::make_unique<EventData>();
                batch_n++;
                packet_counter++;
            }
            last = std::chrono::steady_clock::now();
        } else {
            if (current_retry < max_connection_retries) {
                connectToServer(false);
                int delay = std::min(static_cast<int>(std::pow(2, current_retry) * 100),
                                     static_cast<int>(max_retry_delay));
                std::this_thread::sleep_for(std::chrono::milliseconds(delay));
                current_retry++;
                logger->info("{} trying to connect to server", name);
                continue;
            }
            logger->info("{} had too many failed connections. Stopping.", name);
            run_thread = false;
        }
    }
}

bool SocketSubscriber::getPacket(std::vector<uint8_t>& buffer) const {
    try {
        zmq::message_t msg;
        subscriber->recv(msg, zmq::recv_flags::none);
        size_t sz = msg.size();
        logger->debug("Received packet of size {}", sz);
        buffer.resize(sz);
        std::memcpy(buffer.data(), msg.data(), sz);
        return true;
    }
    catch (const zmq::error_t &e) {
        logger->debug("Error receiving message: {}", e.what());
        return false;
    }
}

void SocketSubscriber::processPacket(std::vector<uint8_t>& buffer) {
    size_t offset = 0;
    uint32_t tag;
    uint16_t l1id, bcid, nHits;
    Hit hit;

    while (offset < buffer.size()) {
        std::memcpy(&tag,   buffer.data() + offset, sizeof(tag));  offset += sizeof(tag);
        std::memcpy(&l1id,  buffer.data() + offset, sizeof(l1id)); offset += sizeof(l1id);
        std::memcpy(&bcid,  buffer.data() + offset, sizeof(bcid)); offset += sizeof(bcid);
        std::memcpy(&nHits, buffer.data() + offset, sizeof(nHits));offset += sizeof(nHits);

        curEvents->newEvent(tag, l1id, bcid);
        total_events++;

        for (uint16_t i = 0; i < nHits; ++i) {
            std::memcpy(&hit, buffer.data() + offset, sizeof(hit));
            offset += sizeof(hit);
            curEvents->addHit(hit);
            total_hits++;
        }
    }
}

bool SocketSubscriber::connectToServer(bool log) {
    try {
        subscriber = std::make_unique<zmq::socket_t>(context, zmq::socket_type::sub);
        // subscribe to all messages
        subscriber->setsockopt(ZMQ_SUBSCRIBE, "", 0);
        std::string endpoint = "tcp://" + server_ip + ":" + port;
        subscriber->connect(endpoint);
        if (log) logger->info("{} connected to {}", name, endpoint);
        is_connected = true;
        return true;
    }
    catch (const zmq::error_t &e) {
        if (log) logger->error("Could not connect to {}:{} - {}", server_ip, port, e.what());
        is_connected = false;
        return false;
    }
}