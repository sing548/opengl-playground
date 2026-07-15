#pragma once

#include <array>
#include <limits>
#include <cstdint>
#include <algorithm>

struct Stat
{
    static constexpr int HISTORY_DURATION = 120;

    int n = 0;
    float sum = 0.0f;
    float min = std::numeric_limits<float>::max();
    float max = std::numeric_limits<float>::lowest();

    uint64_t total = 0;

    struct Snapshot { float rate = 0, mean = 0, min = 0, max = 0; int n = 0; };
    Snapshot last;

    bool isCounter = false;

    std::array<float, HISTORY_DURATION> hist {};
    int histHead  = 0;
    int histCount = 0;

    void Add(float value) 
    { 
        sum += value;
        min = std::min(min, value);
        max = std::max(max, value);

        ++n;
        ++total;
    }

    void Hit()
    {
        Add(1.0f);
        isCounter = true;
    }

    void Flush(float duration)
    {
        last.n     = n;
        last.min   = n > 0 ? min : 0.0f;
        last.max   = n > 0 ? max : 0.0f;
        last.mean  = n > 0 ? sum / n : 0.0f;
        last.rate  = duration > 0.0f ? n / duration : 0.0f;

        hist[histHead] = isCounter ? last.rate : last.mean;
        histHead   = (histHead + 1) % Stat::HISTORY_DURATION;
        histCount  = std::min(histCount + 1, Stat::HISTORY_DURATION);

        n      = 0;
        sum    = 0.0f;
        min    = std::numeric_limits<float>::max();
        max    = std::numeric_limits<float>::lowest();
    }
};
