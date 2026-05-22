#ifndef SERV_TRANSP_H
#define SERV_TRANSP_H

#include <span>
#include <memory>
#include <vector>
#include <cstddef>
#include <cstdint>

using ConnectionId = uint32_t;

class ServTransp
{
public:
    explicit ServTransp(uint16_t port);
    ~ServTransp();

    ServTransp(const ServTransp&) = delete;
    ServTransp& operator=(const ServTransp&) = delete;

    void Send(ConnectionId to, std::span<const std::byte> bytes, bool reliable);
    void Broadcast(std::span<const std::byte> bytes, bool reliable);

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
