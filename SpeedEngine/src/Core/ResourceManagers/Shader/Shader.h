#pragma once

#include "vec2.hpp"
#include "vec3.hpp"
#include "vec4.hpp"
#include "mat2x2.hpp"
#include "mat3x3.hpp"
#include "mat4x4.hpp"

#include"../Resource.h"

namespace SE
{
	//compilation status of shader + its files
	struct GLCompStatus
	{
		bool success { false };
		std::string log { "" };
	};

	class Shader : public Resource
	{
	public:
		Shader(uint32_t id, std::string resourceName);

		//@brief compiles and links a shader object
		//@param vertexShaderSource the literal source code of vertex shader as a string
		//@param fragmentShaderSource the literal source code of fragment shader as a string
		//@returns compilation status containing its success (T/F) and any compilation logs
		GLCompStatus init(std::string_view vertexShaderSource, std::string_view fragmentShaderSource);

		//@brief binds shader to be used in graphics pipeline
		void bind();

		//@brief unbinds shader from use in graphics pipeline
		void unbind();

		//@brief deletes the graphics program
		void destroy();

		// utility uniform functions
		void setBool(std::string_view name, bool value);
		void setInt(std::string_view name, int value);
		void setFloat(std::string_view name, float value);
		void setVec2(std::string_view name, const glm::vec2& value);
		void setVec2(std::string_view name, float x, float y);
		void setVec3(std::string_view name, const glm::vec3& value);
		void setVec3(std::string_view name, float x, float y, float z);
		void setVec4(std::string_view name, const glm::vec4& value);
		void setVec4(std::string_view name, float x, float y, float z, float w);
		void setMat2(std::string_view name, const glm::mat2& mat);
		void setMat3(std::string_view name, const glm::mat3& mat);
		void setMat4(std::string_view name, const glm::mat4& mat);


	private:
		unsigned int m_ProgramId;

		unsigned int _getUniformLocation(std::string_view name);
		std::unordered_map<std::string, unsigned int> m_uniforms;

		GLCompStatus _checkCompilationErr(unsigned int shader, std::string_view type);
	};
}

