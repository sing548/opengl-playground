#pragma once

#include <map>
#include <format>
#include <string>
#include <algorithm>
#include <string_view>

#include "stat.h"

class DebugStats
{
public:

    Stat& operator[](std::string_view name)
    {
        auto it = stats_.find(name);
        if (it == stats_.end())
            it = stats_.try_emplace(std::string(name)).first;
        return it->second;
    }

    template <typename... Args>
    Stat& Parsed(std::format_string<Args...> fmt, Args&&... args)
    {
        char buf[64];
        auto [out, size] = std::format_to_n(buf, sizeof buf, fmt, std::forward<Args>(args)...);
        return (*this)[std::string_view(buf, out - buf)];
    }

    template <typename... Args>
    void Erase(std::format_string<Args...> fmt, Args&&... args)
    {
        char buf[64];
        auto [out, size] = std::format_to_n(buf, sizeof buf, fmt, std::forward<Args>(args)...);

        if (auto it = stats_.find(std::string_view(buf, out - buf)); it != stats_.end())
            stats_.erase(it);
    }

    void Erase(std::string key)
    {
        stats_.erase(key);
    }

    void Flush(float duration)
    {
        for (auto& [name, stat] : stats_)
            stat.Flush(duration);
    }

    const auto& AllStats() const { return stats_; }
private:
    // Like this lookup is stable for display and still fast enough (O(n))
    // compared to unordered_map. string has compare operator
    // no need to do hashes etc
    std::map<std::string, Stat, std::less<>> stats_;
};
