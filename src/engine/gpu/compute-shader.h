#pragma once

#include "shader-base.h"

class ComputeShader : public ShaderBase
{
public:
    ComputeShader(const std::filesystem::path computePath) 
    {
        std::string computeCode = ReadShaderFile(computePath);
        const char* cCode = computeCode.c_str();

        unsigned int compute;
        compute = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(compute, 1, &cCode, NULL);
        glCompileShader(compute);
        CheckCompileErrors(compute, "COMPUTE");

        ID = glCreateProgram();
        glAttachShader(ID, compute);
        glLinkProgram(ID);
        CheckCompileErrors(ID, "PROGRAM");
        glDeleteShader(compute);
    }
};
