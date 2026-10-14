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
		const char* vShaderCode = vertexCode.c_str();
		std::string fragmentCode = ReadShaderFile(fragmentPath); 
		const char* fShaderCode = fragmentCode.c_str();

		unsigned int vertex, fragment = 0;

		try {

			vertex = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vertex, 1, &vShaderCode, NULL);
			glCompileShader(vertex);
			CheckCompileErrors(vertex, "VERTEX");
		}
		catch (std::exception& e)
		{
			std::cout << "Exception" << e.what() << std::endl;
		}

		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		CheckCompileErrors(fragment, "FRAGMENT");

		ID = glCreateProgram();
		glAttachShader(ID, vertex);
		glAttachShader(ID, fragment);
		glLinkProgram(ID);

		CheckCompileErrors(ID, "PROGRAM");

		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}
};

#endif