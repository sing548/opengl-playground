#pragma once 

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

#include <glm/glm.hpp>

class ShaderBase
{
protected:
    ShaderBase() {};

    void CheckCompileErrors(GLuint shader, std::string type)
	{
		GLint success;
		GLchar infoLog[1024];

		if (type != "PROGRAM")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

			if (!success)
			{
				glGetShaderInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR:SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- ------------------- -- " << std::endl;
				throw std::runtime_error("ERROR_:SHADER_COMPILATION_ERROR");
			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);

			if (!success)
			{
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- ----------------------- -- " << std::endl;
				throw std::runtime_error("ERROR_:SHADER_COMPILATION_ERROR");
			}
        }
	};
	
    std::string ReadShaderFile(const std::filesystem::path path)
    {
        std::string code;
        std::ifstream file;

        std::string pathString = path.string();

        file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        try 
        {
            file.open(pathString);
            std::stringstream stream;

            stream << file.rdbuf();

            file.close();

            code = stream.str();
        }
        catch(std::ifstream::failure& e)
        {
            std::cout << "Error during loading of shader: " << pathString << std::endl;
            std::cout << "ERROR::SHADER:FILE_NOT_SUCCESFULLY_READ" << e.what() << std::endl;
            throw std::runtime_error("Shader file read failed");
        }

        return code;
    }
public:
	unsigned int ID;

	void Use()
	{
		glUseProgram(ID);
	}

	void SetBool(const std::string &name, bool value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
	}
	void SetInt(const std::string &name, int value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
	}
	void SetUInt(const std::string &name, uint32_t value) const
	{
		glUniform1ui(glGetUniformLocation(ID, name.c_str()), value);
	}
	void SetFloat(const std::string &name, float value) const
	{
		glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
	}
	void SetVec2(const std::string &name, const glm::vec2 &value) const 
	{
		glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void SetVec2(const std::string &name, float x, float y) const
	{
		glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
	}
	void SetVec2(const std::string &name, uint32_t x, uint32_t y) const
	{
		glUniform2ui(glGetUniformLocation(ID, name.c_str()), x, y);
	}
	void SetVec2(const std::string &name, const glm::ivec2 &value) const
	{
		glUniform2iv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void SetVec3(const std::string &name, const glm::vec3 &value) const
	{
		glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void SetVec3(const std::string &name, float x, float y, float z) const
	{
		glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
	}
	void SetVec3(const std::string &name, uint32_t x, uint32_t y, uint32_t z) const
	{
		glUniform3ui(glGetUniformLocation(ID, name.c_str()), x, y, z);
	}
	void SetVec3(const std::string &name, const glm::ivec3 &value) const
	{
		glUniform3iv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void SetVec4(const std::string &name, const glm::vec4& value) const
	{
		glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void SetVec4(const std::string &name, float x, float y, float z, float w) const
	{
		glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
	}
	void SetMat2(const std::string &name, const glm::mat2 &mat) const
	{
		glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	void SetMat3(const std::string &name, const glm::mat3 &mat) const
	{
		glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	void SetMat4(const std::string& name, const glm::mat4& mat) const
	{
		glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}	
};
