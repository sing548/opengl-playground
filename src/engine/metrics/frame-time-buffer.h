#pragma once

#include <array>
#include <algorithm>

struct FrameTimeBuffer
{
    static constexpr int CAPACITY = 300;

    std::array<float, CAPACITY> samples {};
    int head = 0;
    int count = 0;
    float pending = 0.0f;
    float scaleMax = 33.0f;

    void Accumulate(float v) { pending += v; }
    void Set (float v) { pending = v; }

    void Commit() 
    {
        samples[head] = pending;
        head = (head + 1) % CAPACITY;
        count = std::min(count + 1, CAPACITY);
        pending = 0.0f;
    }

    int Offset() const { return count < CAPACITY ? 0 : head; }
    float Max() const { return count == 0 ? 0.0f : *std::max_element(samples.begin(), samples.begin() + count); }
};
