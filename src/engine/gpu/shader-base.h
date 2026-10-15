#pragma once 

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <unordered_map>

#include <glm/glm.hpp>

class ShaderBase
{
protected:
    ShaderBase() {};
	~ShaderBase() 
	{
		glDeleteProgram(ID);
	}
	ShaderBase(const ShaderBase& other) = delete;
	ShaderBase& operator=(const ShaderBase&) = delete;

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

	GLint Location(const std::string& name) const
	{
		auto it = locations_.find(name);

		if (it != locations_.end())
			return it->second;

		GLint loc = glGetUniformLocation(ID, name.c_str());

		if (loc == -1)
			std::cerr << "Shader: " << ID << ": unknown uniform '" << name << "'\n";
		
		locations_.emplace(name, loc);
		return loc;
	}

	GLuint CompileStage(GLenum type, const std::string& code, const char* label)
	{
		const char* src = code.c_str();

		GLuint stage = glCreateShader(type);
		glShaderSource(stage, 1, &src, nullptr);
		glCompileShader(stage);

		try {
			CheckCompileErrors(stage, label);
		}
		catch (std::exception& e)
		{
			glDeleteShader(stage);
			throw;
		}

		return stage;
	}

	mutable std::unordered_map<std::string, GLint> locations_;
public:
	unsigned int ID = 0;

	void Use()
	{
		glUseProgram(ID);
	}

	void SetBool(const std::string &name, bool value) const
	{
		glProgramUniform1i(ID, Location(name), (int)value);
	}
	void SetInt(const std::string &name, int value) const
	{
		glProgramUniform1i(ID, Location(name), value);
	}
	void SetUInt(const std::string &name, uint32_t value) const
	{
		glProgramUniform1ui(ID, Location(name), value);
	}
	void SetFloat(const std::string &name, float value) const
	{
		glProgramUniform1f(ID, Location(name), value);
	}
	void SetVec2(const std::string &name, const glm::vec2 &value) const 
	{
		glProgramUniform2fv(ID, Location(name), 1, &value[0]);
	}
	void SetVec2(const std::string &name, float x, float y) const
	{
		glProgramUniform2f(ID, Location(name), x, y);
	}
	void SetVec2(const std::string &name, uint32_t x, uint32_t y) const
	{
		glProgramUniform2ui(ID, Location(name), x, y);
	}
	void SetVec2(const std::string &name, const glm::ivec2 &value) const
	{
		glProgramUniform2iv(ID, Location(name), 1, &value[0]);
	}
	void SetVec3(const std::string &name, const glm::vec3 &value) const
	{
		glProgramUniform3fv(ID, Location(name), 1, &value[0]);
	}
	void SetVec3(const std::string &name, float x, float y, float z) const
	{
		glProgramUniform3f(ID, Location(name), x, y, z);
	}
	void SetVec3(const std::string &name, uint32_t x, uint32_t y, uint32_t z) const
	{
		glProgramUniform3ui(ID, Location(name), x, y, z);
	}
	void SetVec3(const std::string &name, const glm::ivec3 &value) const
	{
		glProgramUniform3iv(ID, Location(name), 1, &value[0]);
	}
	void SetVec4(const std::string &name, const glm::vec4& value) const
	{
		glProgramUniform4fv(ID, Location(name), 1, &value[0]);
	}
	void SetVec4(const std::string &name, float x, float y, float z, float w) const
	{
		glProgramUniform4f(ID, Location(name), x, y, z, w);
	}
	void SetMat2(const std::string &name, const glm::mat2 &mat) const
	{
		glProgramUniformMatrix2fv(ID, Location(name), 1, GL_FALSE, &mat[0][0]);
	}
	void SetMat3(const std::string &name, const glm::mat3 &mat) const
	{
		glProgramUniformMatrix3fv(ID, Location(name), 1, GL_FALSE, &mat[0][0]);
	}
	void SetMat4(const std::string& name, const glm::mat4& mat) const
	{
		glProgramUniformMatrix4fv(ID, Location(name), 1, GL_FALSE, &mat[0][0]);
	}	
};
