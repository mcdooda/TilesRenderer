#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <GL/glew.h>

class GLProgram
{
public:
	GLProgram()
		: m_programId(0)
	{

	}

	~GLProgram()
	{
		if (glIsProgram(m_programId))
		{
			glDeleteProgram(m_programId);
		}
	}

	void load(const std::string& fragmentShader, const std::string& vertexShader)
	{
		m_fragmentShader = fragmentShader;
		m_vertexShader = vertexShader;

		const GLuint fragmentShaderId = compileShader(fragmentShader, GL_FRAGMENT_SHADER);
		const GLuint vertexShaderId = compileShader(vertexShader, GL_VERTEX_SHADER);
		m_programId = compileProgram(fragmentShaderId, vertexShaderId);
	}

	void use() const
	{
		glUseProgram(m_programId);
	}

	inline GLuint getProgramId() const { return m_programId; }

protected:
	GLuint compileProgram(GLuint fragmentShaderId, GLuint vertexShaderId)
	{
		GLuint programId = glCreateProgram();
		glAttachShader(programId, vertexShaderId);
		glAttachShader(programId, fragmentShaderId);
		glLinkProgram(programId);

		char buffer[8192];
		GLsizei length = 0;
		glGetProgramInfoLog(programId, sizeof(buffer), &length, buffer);
		if (length)
		{
			printf("%s\n", buffer);
			assert(false);
		}

		return programId;
	}

	GLuint compileShader(const std::string& shader, GLuint shaderType)
	{
		std::string shaderCode;
		readCode(shader, shaderCode);
		GLuint shaderId = glCreateShader(shaderType);
		const GLchar* p = shaderCode.c_str();
		glShaderSource(shaderId, 1, &p, nullptr);
		glCompileShader(shaderId);
		return shaderId;
	}

	void readCode(const std::string& shader, std::string& code)
	{
		std::ifstream file(shader.c_str(), std::ifstream::binary);
		if (!file.is_open())
		{
			std::cerr << "Warning: unable to open shader file '" << shader << "'" << std::endl;
			code = "";
		}

		std::stringstream buffer;
		buffer << file.rdbuf();
		code = buffer.str();

		file.close();
	}

protected:
	std::string m_fragmentShader;
	std::string m_vertexShader;

	GLuint m_programId;
};