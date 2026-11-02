#include "pch.h"

#include "TextRenderer.h"
#include "Core/Engine.h"
#include "Core/ResourceManagers/ResourceManager.h"
#include "Core/SubSystems/Graphics/Rendering/RenderContext.h"

#include <glad/glad.h>

namespace SE
{
	void TextRenderer::init()
	{
		auto* resources = Engine::Instance().getSubSystem<ResourceManager>();
		const auto* vertex = resources->getString("Assets/Shaders/text.vert");
		const auto* fragment = resources->getString("Assets/Shaders/text.frag");
		if (!vertex || !fragment)
		{
			m_logger.error("Text shaders are unavailable");
			return;
		}
		const auto status = m_shader.init(*vertex, *fragment);
		if (!status.success)
		{
			m_logger.error("Text shader compilation failed: {}", status.log);
			m_shader.destroy();
			return;
		}
		GLint previousVAO = 0, previousBuffer = 0;
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previousVAO);
		glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &previousBuffer);
		glGenVertexArrays(1, &m_VAO);
		glGenBuffers(1, &m_VBO);
		if (!m_VAO || !m_VBO)
		{
			m_logger.error("Failed to create text vertex buffers");
			shutdown();
			return;
		}
		glBindVertexArray(m_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, uv)));
		glBindVertexArray(previousVAO);
		glBindBuffer(GL_ARRAY_BUFFER, previousBuffer);
	}

	unsigned int TextRenderer::_getOrUpload(const FontHandle& font)
	{
		if (auto it = m_atlases.find(font); it != m_atlases.end())
			return it->second;

		const GLenum unpackNames[] = { GL_UNPACK_ALIGNMENT, GL_UNPACK_ROW_LENGTH, GL_UNPACK_SKIP_ROWS,
			GL_UNPACK_SKIP_PIXELS, GL_PIXEL_UNPACK_BUFFER_BINDING };
		GLint unpack[5]{};
		for (size_t i = 0; i < std::size(unpackNames); ++i)
			glGetIntegerv(unpackNames[i], &unpack[i]);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);

		unsigned int texture = 0;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, font->atlasWidth, font->atlasHeight, 0,
			GL_RED, GL_UNSIGNED_BYTE, font->atlasPixels.data());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		for (size_t i = 0; i < 4; ++i)
			glPixelStorei(unpackNames[i], unpack[i]);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, unpack[4]);
		m_atlases.emplace(font, texture);
		return texture;
	}

	void TextRenderer::render(RenderContext& context)
	{
		if (!m_VAO || !context.output || context.frame.textDraws.empty())
			return;

		GLint previousTexture = 0, previousBuffer = 0, previousSampler = 0;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousTexture);
		glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &previousBuffer);
		glGetIntegerv(GL_SAMPLER_BINDING, &previousSampler);
		const GLenum blendNames[] = { GL_BLEND_EQUATION_RGB, GL_BLEND_EQUATION_ALPHA,
			GL_BLEND_SRC_RGB, GL_BLEND_DST_RGB, GL_BLEND_SRC_ALPHA, GL_BLEND_DST_ALPHA };
		GLint blend[6]{};
		for (size_t i = 0; i < std::size(blendNames); ++i)
			glGetIntegerv(blendNames[i], &blend[i]);

		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		glDisable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glBindSampler(0, 0);
		m_shader.bind();
		m_shader.setInt("glyphAtlas", 0);
		m_shader.setVec2("outputSize", glm::vec2(context.output->getWidth(), context.output->getHeight()));
		glBindVertexArray(m_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

		for (const auto& draw : context.frame.textDraws)
		{
			if (!draw.font || draw.color.a <= 0.0f)
				continue;
			m_vertices.clear();
			draw.font->layout(draw.text, [&](const FontGlyph& glyph, const glm::vec2& topLeft)
			{
				const glm::vec2 a = draw.position + topLeft;
				const glm::vec2 b = a + glyph.size;
				m_vertices.insert(m_vertices.end(), {
					{ a, glyph.uvMin },
					{ { b.x, a.y }, { glyph.uvMax.x, glyph.uvMin.y } },
					{ b, glyph.uvMax },
					{ a, glyph.uvMin },
					{ b, glyph.uvMax },
					{ { a.x, b.y }, { glyph.uvMin.x, glyph.uvMax.y } }
				});
			});
			if (m_vertices.empty())
				continue;
			glBindTexture(GL_TEXTURE_2D, _getOrUpload(draw.font));
			m_shader.setVec4("textColor", draw.color);
			glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), m_vertices.data(), GL_STREAM_DRAW);
			glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_vertices.size()));
		}

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, previousBuffer);
		glBindTexture(GL_TEXTURE_2D, previousTexture);
		glBindSampler(0, previousSampler);
		m_shader.unbind();
		glBlendEquationSeparate(blend[0], blend[1]);
		glBlendFuncSeparate(blend[2], blend[3], blend[4], blend[5]);
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glEnable(GL_CULL_FACE);
	}

	void TextRenderer::shutdown()
	{
		for (const auto& [font, texture] : m_atlases)
			glDeleteTextures(1, &texture);
		m_atlases.clear();
		glDeleteBuffers(1, &m_VBO);
		glDeleteVertexArrays(1, &m_VAO);
		m_VBO = m_VAO = 0;
		m_vertices.clear();
		m_shader.destroy();
	}
} // namespace SE
