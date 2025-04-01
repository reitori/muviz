#include "include/SocketReceiver.h"
#include "logging.h"

namespace
{
    auto logger = logging::make_log("SocketReceiver");
    bool YarrBinaryFileRegistered =
      StdDict::registerDataLoader("SocketReceiver",
                                []() { return std::unique_ptr<DataLoader>(new SocketReceiver());});
}

SocketReceiver::SocketReceiver() {
    total_events = 0;
    total_hits = 0;
    current_retry = 0;
    max_retry_delay = 60000;
    max_connection_retries = 10;
    run_thread = false;
}

SocketReceiver::~SocketReceiver() {}

void SocketReceiver::run(){
    run_thread = true;
    thread_ptr.reset(new std::thread(&SocketReceiver::process, this));
}

void SocketReceiver::join() {
    run_thread = false;
    thread_ptr->join();
    logger->info("[{}]: Processed {} events, with {} hits, in {} batches", name, total_events, total_hits, packet_counter);
}

void SocketReceiver::init() {
    total_events = 0;
    total_hits = 0;
    current_retry = 0;
    run_thread = false;
    is_connected = false;

    connectToServer();
}


void SocketReceiver::configure(const json &config){
    if(config.contains("name")) {
        name = config["name"]; 
    }
    else {
        logger->error("No name provided! Please add name to all sources. Assuming name = None");
        throw(std::invalid_argument("No name provided!"));
    }

    if(config.contains("server_ip")){
        server_ip = (std::string)config["server_ip"];
    }
    else{
        logger->warn("\"server_ip\" not provided. Assuming local host.");
        server_ip = "127.0.0.1"; // (local host) assume client is also server
    }

    if(config.contains("port")){
        port = config["port"].get<std::string>();
    }
    else{
        logger->error("No port is provided for {0}", name);
        throw(std::invalid_argument("No port provided!"));
    }

    if(config.contains("max_connection_retries"))
    max_connection_retries = config["max_connection_retries"];
    else max_connection_retries = 10;

    if(config.contains("max_retry_delay"))
    max_retry_delay = config["max_retry_delay"];
    else max_retry_delay = 60000;
}


void SocketReceiver::process(){
    uint32_t batch_n = 0;
    curEvents = std::make_unique<EventData>();

    std::chrono::steady_clock::time_point last = std::chrono::steady_clock::now(), now;

    while(run_thread) {
        if(is_connected){
            std::vector<uint8_t> rawbytes;
            if(!getPacket(rawbytes)){
                logger->debug("Failed to get {0} packet", packet_counter);
                continue;
            }
            processPacket(rawbytes);
        }else{ //retry connection
            if(current_retry < max_connection_retries){
                connectToServer(false);
                int delay = std::min(static_cast<int>(pow(2, current_retry) * 100), (int)max_retry_delay);
                std::this_thread::sleep_for(std::chrono::milliseconds(delay));
                current_retry++;

                logger->info("{0} trying to connect to server", name);

                continue;
            }

            logger->info("{0} had too many failed connections. Closing port.", name);
            run_thread = false; //too many retries
        }
        now = std::chrono::steady_clock::now();
        if(curEvents->size() > 0) {
            auto diff = ((float)std::chrono::duration_cast<std::chrono::microseconds>(now - last).count())/1000000;
            logger->debug(
                "[{}] Packet {}: {} events in {} seconds = {} ev/s", 
                name, batch_n, curEvents->size(), diff, curEvents->size()/diff
            );
            // Push data and make new block of events
            output->pushData(std::move(curEvents));
            curEvents = std::make_unique<EventData>();

            batch_n++;
        }
        last = std::chrono::steady_clock::now();
    }
}

bool SocketReceiver::getPacket(std::vector<uint8_t>& buffer) const{
    uint32_t packetSize = 0;
    uint32_t bytesRead = 0;

    while (bytesRead < sizeof(uint32_t)) {
        int result = recv(fd, ((char*)&packetSize) + bytesRead, sizeof(uint32_t) - bytesRead, 0);
        if (result <= 0) return false;
        bytesRead += result;
    }

    packetSize = ntohl(packetSize);
    size_t offset = buffer.size();
    buffer.resize(buffer.size() + packetSize);
    bytesRead = 0;
    
    while(bytesRead < packetSize){
        int nbytes = recv(fd, buffer.data() + offset + bytesRead, packetSize - bytesRead, 0);
        if(nbytes <= 0) return false;
        bytesRead += nbytes;
    }
    return true;
}

void SocketReceiver::processPacket(std::vector<uint8_t>& buffer){
    size_t offset = 0;

    uint32_t tag;
    uint16_t l1id, bcid, nHits;
    Hit hit;

    while(offset < buffer.size()){
        std::memcpy(&tag, buffer.data() + offset, sizeof(uint32_t)); offset += sizeof(uint32_t);
        std::memcpy(&l1id, buffer.data() + offset, sizeof(uint16_t)); offset += sizeof(uint16_t);
        std::memcpy(&bcid, buffer.data() + offset, sizeof(uint16_t)); offset += sizeof(uint16_t);
        std::memcpy(&nHits, buffer.data() + offset, sizeof(uint16_t)); offset += sizeof(uint16_t);

        curEvents->newEvent(tag, l1id, bcid);

        for(uint16_t i = 0; i < nHits; i++){
            std::memcpy(&hit, buffer.data() + offset, sizeof(hit)); offset += sizeof(Hit);
            curEvents->addHit(hit);
            total_hits++;
        }
    }

    packet_counter++;
}

bool SocketReceiver::connectToServer(bool log = true){
    int rv;
    struct addrinfo hints, *servinfo, *p;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    if ((rv = getaddrinfo(server_ip.c_str(), port.c_str(), &hints, &servinfo)) != 0) {
        if(log) logger->error("Could not get server info: {0}", gai_strerror(rv));
        return false;
    }

    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((fd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) {
            if(log) logger->debug("{1} tried to create socket with address family {0} and failed", p->ai_family, name);
            continue;
        }

        if (::connect(fd, p->ai_addr, p->ai_addrlen) == -1) {
            close(fd);
            if(log) logger->debug("{1} tried to connect with address family {0} and failed", p->ai_family, name);
            continue;
        }

        break;
    }
    freeaddrinfo(servinfo);

    if (p == NULL) {
        if(log) logger->error("{0} failed to connect to server", name);
        return false;
    }

    if(log) logger->info("{0} connected to {1}", name, server_ip);
    is_connected = true;

    return true;
}

const char* SocketReceiver::getIP(int arg_fd) const{
    struct sockaddr_storage addr;
    socklen_t addr_len = sizeof(addr);

    if (getpeername(arg_fd, (struct sockaddr*)&addr, &addr_len) == -1) {
        logger->debug("Could not get IP of fd {0}", arg_fd);
        return;
    }

    return getIP(addr);
}

const char* SocketReceiver::getIP(struct sockaddr_storage& addr) const{
    char ipstr[INET6_ADDRSTRLEN];
    if (addr.ss_family == AF_INET) { // IPv4
        struct sockaddr_in *s = (struct sockaddr_in *)&addr;
        inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
    } else { // IPv6
        struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
        inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
    }

    return ipstr;
}
