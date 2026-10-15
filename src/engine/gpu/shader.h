#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

#include <glm/glm.hpp>

#include "shader-base.h"

class Shader : public ShaderBase
{
public:
	Shader(const std::filesystem::path vertexPath, const std::filesystem::path fragmentPath)
	{
		std::string vertexCode = ReadShaderFile(vertexPath);
		std::string fragmentCode = ReadShaderFile(fragmentPath);

		GLuint vertex = CompileStage(GL_VERTEX_SHADER, vertexCode, "VERTEX");
		GLuint fragment = 0;

		try
		{
			fragment = CompileStage(GL_FRAGMENT_SHADER, fragmentCode, "FRAGMENT");
		}
		catch (std::exception& e)
		{
			glDeleteShader(vertex);
			throw;
		}

		ID = glCreateProgram();
		glAttachShader(ID, vertex);
		glAttachShader(ID, fragment);
		glLinkProgram(ID);

		glDeleteShader(vertex);
		glDeleteShader(fragment);

		CheckCompileErrors(ID, "PROGRAM");
	}
};

#endif