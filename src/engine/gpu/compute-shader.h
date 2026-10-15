#pragma once

#include "shader-base.h"

class ComputeShader : public ShaderBase
{
public:
    ComputeShader(const std::filesystem::path computePath) 
    {
        GLuint compute = CompileStage(GL_COMPUTE_SHADER, ReadShaderFile(computePath), "COMPUTE");

        ID = glCreateProgram();
        glAttachShader(ID, compute);
        glLinkProgram(ID);
        glDeleteShader(compute);
        
        CheckCompileErrors(ID, "PROGRAM");

        GLint size[3];
        glGetProgramiv(ID, GL_COMPUTE_WORK_GROUP_SIZE, size);
        groupSize_ = static_cast<uint32_t>(size[0]);
    }

    void DispatchFor(uint32_t n)
    {
        Use();
        glDispatchCompute((n + groupSize_ - 1) / groupSize_, 1, 1);
    }

    void Dispatch(uint32_t x, uint32_t y = 1, uint32_t z = 1)
    {
        Use();
        glDispatchCompute(x, y, z);
    }
private:
    uint32_t groupSize_ = 1;
};
