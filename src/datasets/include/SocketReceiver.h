#ifndef SOCKETRECEIVER_H
#define SOCKETRECEIVER_H

#include "AllDataLoaders.h"

#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include <cstring>
#include <vector>
#include <algorithm>

namespace viz{
    class SocketReceiver : public DataLoader{
    public:
        SocketReceiver();
        ~SocketReceiver();
        
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

        std::string getIP(int arg_fd) const;
        std::string getIP(struct sockaddr_storage& addr) const;

        std::string name, server_ip, port;
        int connections;

        unsigned total_events, packet_counter, total_hits, max_retry_delay;
        bool run_thread, is_connected, socket_created;
        int fd;

        uint32_t totalEvents = 0;
        uint8_t max_connection_retries, current_retry;
        std::unique_ptr<EventData> curEvents;
    };
}

#endif