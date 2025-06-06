// SocketSubscriber.h
#ifndef SOCKETSUBSCRIBER_H
#define SOCKETSUBSCRIBER_H

#include "AllDataLoaders.h"
#include <zmq.hpp>
#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <thread>
#include <cmath>
#include <cstring>

#include <iostream>
class SocketSubscriber : public DataLoader {
public:
    SocketSubscriber();
    ~SocketSubscriber() override;

    // interface
    void init() override;
    void configure(const json &config) override;
    void run() override;
    void join() override;

private:
    void process();
    bool getPacket(std::vector<uint8_t>& buffer) const;
    void processPacket(std::vector<uint8_t>& buffer);

    bool connectToServer(bool log = true);

    std::string name, server_ip, port;

    unsigned   total_events, packet_counter, total_hits, max_retry_delay;
    uint8_t    max_connection_retries, current_retry;
    bool       run_thread, is_connected;

    std::unique_ptr<EventData>   curEvents;
    std::unique_ptr<std::thread> thread_ptr;

    zmq::context_t                    context;
    std::unique_ptr<zmq::socket_t>    subscriber;
};

#endif