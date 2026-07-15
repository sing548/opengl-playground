#ifndef CLI_TRANSP_H
#define CLU_TRANSP_H

#include <span>
#include <string>
#include <vector>
#include <memory>
#include <cstddef>

class ClientTransport
{
public:
    explicit ClientTransport(const std::string& serverAddr);
    ~ClientTransport();

    ClientTransport(const ClientTransport&) = delete;
    ClientTransport& operator=(const ClientTransport&) = delete;

    void Send(std::span<const std::byte> bytes, bool reliable, bool noNagle = false);
    
    static void SetFakeNetwork(int lagMs, float pkgLossPct, float pkgJitterPct);

    struct Event
    {
        enum class Kind { Connected, Disconnected, Message };
        Kind kind;
        std::vector<std::byte> bytes;
    };
    std::vector<Event> PollEvents();

    struct Impl;

private:
    std::unique_ptr<Impl> impl_;
};

#endif