#pragma once

#include <glad/glad.h>

#include <vector>
#include <cstddef>
#include <utility>

class GPUBuffer
{
public:
    GPUBuffer() = default;

    explicit GPUBuffer(size_t bytes, const void* data = nullptr, GLenum usage = GL_DYNAMIC_DRAW) : usage_(usage)
    {
        Allocate(bytes, data);
    }

    template<typename T>
    explicit GPUBuffer(const std::vector<T>& data, GLenum usage = GL_DYNAMIC_DRAW) 
        : GPUBuffer(data.size() * sizeof(T), data.data(), usage) {}

    ~GPUBuffer()
    {
        glDeleteBuffers(1, &id_);
    };

    GPUBuffer(const GPUBuffer&) = delete;
    GPUBuffer& operator=(const GPUBuffer&) = delete;

    GPUBuffer(GPUBuffer&& other) noexcept
        : id_(std::exchange(other.id_, 0)),
          size_(std::exchange(other.size_, 0)),
          usage_(other.usage_) {}

    GPUBuffer& operator=(GPUBuffer&& other) noexcept
    {
        if (this != &other)
        {
            glDeleteBuffers(1, &id_);
            id_ = std::exchange(other.id_, 0);
            size_ = std::exchange(other.size_, 0);
            usage_ = other.usage_;
        }

        return *this;
    }

    GLuint Id() const
    {
        return id_;
    }

    size_t Size() const
    {
        return size_;
    }

    void Upload(const void* data, size_t bytes)
    {
        if (id_ == 0 || bytes != size_)
            Allocate(bytes, data);
        else
            glNamedBufferSubData(id_, 0, static_cast<GLsizeiptr>(bytes), data);
    }

    template<typename T>
    void Upload(const std::vector<T>& data)
    {
        Upload(data.data(), data.size() * sizeof(T));
    }

    void BindBase(GLuint slot) const
    {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, slot, id_);
    }

    void Resize(size_t bytes)
    {
        if (id_ == 0 || bytes != size_)
            Allocate(bytes, nullptr);
    }

    template<typename T>
    void ReadData(std::vector<T>& out) const
    {
        out.resize(size_ / sizeof(T));
        glGetNamedBufferSubData(id_, 0, static_cast<GLsizeiptr>(out.size() * sizeof(T)), out.data());
    }

    void SetUsage(GLenum usage) { usage_ = usage; }
private:
    GLuint id_      = 0;
    size_t size_    = 0;
    GLenum usage_   = GL_DYNAMIC_DRAW;

    void Allocate(size_t bytes, const void* data)
    {
        if (id_ == 0)
            glCreateBuffers(1, &id_);

        glNamedBufferData(id_, static_cast<GLsizeiptr>(bytes), data, usage_);
        //glNamedBufferStorage(id_, bytes, data, GL_DYNAMIC_STORAGE_BIT);
        size_ = bytes;
    }
};