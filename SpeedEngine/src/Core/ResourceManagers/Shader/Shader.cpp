#include "pch.h"
#include "Shader.h"
#include "glad/gl.h"

namespace SE
{
	Shader::Shader(uint32_t resourceId, std::string resourceName) : Resource(resourceId, resourceName), m_ProgramId(0)
	{
	}

	GLCompStatus Shader::init(std::string_view vertexShaderSource, std::string_view fragmentShaderSource)
	{
		GLCompStatus status;

		// Compile vertex shader
		unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
		const char* vertexSourceCStr = vertexShaderSource.data();
		glShaderSource(vertexShader, 1, &vertexSourceCStr, nullptr);
		glCompileShader(vertexShader);
		status = _checkCompilationErr(vertexShader, "VERTEX");
		if (!status.success) return status;

		// Compile fragment shader
		unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		const char* fragmentSourceCStr = fragmentShaderSource.data();
		glShaderSource(fragmentShader, 1, &fragmentSourceCStr, nullptr);
		glCompileShader(fragmentShader);
		status = _checkCompilationErr(fragmentShader, "FRAGMENT");
		if (!status.success) return status;

		// Link shaders into a program
		m_ProgramId = glCreateProgram();
		glAttachShader(m_ProgramId, vertexShader);
		glAttachShader(m_ProgramId, fragmentShader);
		glLinkProgram(m_ProgramId);
		status = _checkCompilationErr(m_ProgramId, "PROGRAM");
		if (!status.success) return status;

		// Clean up shaders as they are no longer needed after linking
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		return status;
	}

	const void Shader::bind()
	{
		glUseProgram(m_ProgramId);
	}
	void Shader::unbind()
	{
		glUseProgram(0);
	}
	const void Shader::destroy()
	{
		glDeleteProgram(m_ProgramId);
	}

	GLCompStatus Shader::_checkCompilationErr(unsigned int shader, std::string_view type)
	{
		int success;
		char infoLog[1024];
		if (type != "PROGRAM")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
			return { bool(success), infoLog };
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
			return { bool(success), infoLog };
		}
	}

	unsigned int Shader::_getUniformLocation(std::string_view name)
	{
		if (m_uniforms.find(name.data()) != m_uniforms.end())
		{
			return m_uniforms[name.data()];
		}
		else
		{
			unsigned int location = glGetUniformLocation(m_ProgramId, name.data());
			m_uniforms[name.data()] = location;
			return location;
		}
	}

	void Shader::setBool(std::string_view name, bool value)
	{
		glUniform1i(_getUniformLocation(name), (int)value);
	}
	void Shader::setInt(std::string_view name, int value)
	{
		glUniform1i(_getUniformLocation(name), value);
	}
	void Shader::setFloat(std::string_view name, float value)
	{
		glUniform1f(_getUniformLocation(name), value);
	}
	void Shader::setVec2(std::string_view name, const glm::vec2& value)
	{
		glUniform2fv(_getUniformLocation(name), 1, &value[0]);
	}
	void Shader::setVec2(std::string_view name, float x, float y)
	{
		glUniform2f(_getUniformLocation(name), x, y);
	}
	void Shader::setVec3(std::string_view name, const glm::vec3& value)
	{
		glUniform3fv(_getUniformLocation(name), 1, &value[0]);
	}
	void Shader::setVec3(std::string_view name, float x, float y, float z)
	{
		glUniform3f(_getUniformLocation(name), x, y, z);
	}
	void Shader::setVec4(std::string_view name, const glm::vec4& value)
	{
		glUniform4fv(_getUniformLocation(name), 1, &value[0]);
	}
	void Shader::setVec4(std::string_view name, float x, float y, float z, float w)
	{
		glUniform4f(_getUniformLocation(name), x, y, z, w);
	}
}
