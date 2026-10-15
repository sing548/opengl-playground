#pragma once

#include <glad/glad.h>

#include <stdexcept>

class GPUTimer
{
public:
    GPUTimer() 
    {
        glCreateQueries(GL_TIME_ELAPSED, 1, &query_);
    }
    ~GPUTimer()
    {
        glDeleteQueries(1, &query_);
    }
    GPUTimer(const GPUTimer&) = delete;
    GPUTimer operator=(const GPUTimer&) = delete;

    void StartTimer()
    {
        glBeginQuery(GL_TIME_ELAPSED, query_);
        running_ = 1;
    }

    void StopTimer()
    {
        glEndQuery(GL_TIME_ELAPSED);
        running_ = 0;
    }

    float GetResultMS()
    {
        GLuint64 ns = 0;

        if (running_ == -1)
            throw std::runtime_error("GPUTimer query result asked for null query");

        if (running_ == 1)
            StopTimer();

        glGetQueryObjectui64v(query_, GL_QUERY_RESULT, &ns);

        return static_cast<float>(ns) / 1.0e6f;
    }
private:
    int running_ = -1;
    GLuint query_ = 0;
};
