#ifndef CLI_TRANSP_H
#define CLU_TRANSP_H

#include <span>
#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <cstddef>

class CliTransp
{
public:
    explicit CliTransp(const std::string& serverAddr);
    ~CliTransp();

    CliTransp(const CliTransp&) = delete;
    CliTransp& operator=(const CliTransp&) = delete;

    void Send(std::span<const std::byte> bytes, bool reliable);

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