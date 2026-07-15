#ifndef SERV_TRANSP_H
#define SERV_TRANSP_H

#include <span>
#include <memory>
#include <vector>
#include <cstddef>
#include <cstdint>

using ConnectionId = uint32_t;

class ServerTransport
{
public:
    explicit ServerTransport(uint16_t port);
    ~ServerTransport();

    ServerTransport(const ServerTransport&) = delete;
    ServerTransport& operator=(const ServerTransport&) = delete;

    void Send(ConnectionId to, std::span<const std::byte> bytes, bool reliable, bool noNagle = false);
    void Broadcast(std::span<const std::byte> bytes, bool reliable, bool noNagle = false);

    static void SetFakeNetwork(int lagMs, float pkgLossPct, float pkgJitterPct);

    struct Event
    {
        enum class Kind { Connected, Disconnected, Message };
        Kind kind;
        ConnectionId conn;
        std::vector<std::byte> bytes;
    };

    std::vector<Event> PollEvents();
    
    struct Impl;

private:
    std::unique_ptr<Impl> impl_;
};

#endif
