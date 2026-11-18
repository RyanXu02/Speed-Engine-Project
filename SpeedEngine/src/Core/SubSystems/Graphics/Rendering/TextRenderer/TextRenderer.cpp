#include "pch.h"

#include "TextRenderer.h"
#include "Core/Engine.h"
#include "Core/ResourceManagers/ResourceManager.h"
#include "Core/SubSystems/Graphics/Rendering/RenderContext.h"
#include "Core/SubSystems/UI/UILayout.h"

#include <glad/glad.h>

namespace SE
{
	void TextRenderer::init()
	{
		auto* resources = Engine::Instance().getSubSystem<ResourceManager>();
		const auto status = loadStartupShader(m_shader, *resources, { "Assets/Shaders/text.vert", "Assets/Shaders/text.frag", "text" });
		if (!status.success)
		{
			m_logger.error("Text shader compilation failed: {}", status.log);
			shutdown();
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

	unsigned int TextRenderer::_getOrUpload(ResourceID id, const FontData& font)
	{
		if (auto it = m_atlases.find(id); it != m_atlases.end())
			return it->second;

		unsigned int texture = 0;
		glGenTextures(1, &texture);
		if (!texture)
		{
			m_logger.error("Failed to create font atlas texture");
			return 0;
		}

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

		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, font.atlasWidth, font.atlasHeight, 0,
			GL_RED, GL_UNSIGNED_BYTE, font.atlasPixels.data());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		for (size_t i = 0; i < 4; ++i)
			glPixelStorei(unpackNames[i], unpack[i]);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, unpack[4]);
		m_atlases.emplace(id, texture);
		return texture;
	}

	void TextRenderer::_appendQuad(glm::vec2 a, glm::vec2 b, glm::vec2 uvMin, glm::vec2 uvMax)
	{
		m_vertices.insert(m_vertices.end(), {
			{ a, uvMin },
			{ { b.x, a.y }, { uvMax.x, uvMin.y } },
			{ b, uvMax },
			{ a, uvMin },
			{ b, uvMax },
			{ { a.x, b.y }, { uvMin.x, uvMax.y } }
		});
	}

	void TextRenderer::_drawVertices(unsigned int texture, const glm::vec4& color, int mode)
	{
		glBindTexture(GL_TEXTURE_2D, texture);
		m_shader.setInt("drawMode", mode);
		m_shader.setVec4("textColor", color);
		glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), m_vertices.data(), GL_STREAM_DRAW);
		glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_vertices.size()));
	}

	void TextRenderer::_drawText(ResourceID fontId, const FontData& font, std::string_view text, glm::vec2 position, glm::vec4 color, float scale)
	{
		if (color.a <= 0.0f)
			return;
		m_vertices.clear();
		font.layout(text, [&](const FontGlyph& glyph, const glm::vec2& topLeft)
		{
			const glm::vec2 a = position + topLeft * scale;
			_appendQuad(a, a + glyph.size * scale, glyph.uvMin, glyph.uvMax);
		});
		if (m_vertices.empty())
			return;
		const unsigned int atlas = _getOrUpload(fontId, font);
		if (atlas)
			_drawVertices(atlas, color, 0);
	}

	void TextRenderer::_drawRectangle(const UIRectDraw& draw, glm::vec2 outputSize, RenderContext& context)
	{
		const auto texture = draw.texture ? context.gpu.texture(context.resources, draw.texture) : 0;
		if (draw.texture && !texture)
			return;
		const auto canvas = resolveUICanvasLayout(outputSize, draw.referenceResolution);
		const auto rect = resolveUIRectLayout(draw.rect, canvas);
		m_vertices.clear();

		_appendQuad(rect.pixelTopLeft, rect.pixelTopLeft + rect.pixelSize, { 0.0f, 1.0f }, { 1.0f, 0.0f });
		_drawVertices(texture, draw.color, draw.texture ? 2 : 1);
	}

	void TextRenderer::_drawUIText(const UITextDraw& draw, glm::vec2 outputSize, ResourceManager& resources)
	{
		const auto* font = resources.getResource<FontData>(draw.font);
		if (!font)
			return;
		const auto canvas = resolveUICanvasLayout(outputSize, draw.referenceResolution);
		const auto rect = resolveUIRectLayout(draw.rect, canvas);
		const float scale = draw.fontSize / font->pixelHeight * canvas.scale;
		const glm::vec2 extent = font->measure(draw.text).extent * scale;
		const float horizontal = draw.horizontalAlignment == UIHorizontalAlignment::Center ? 0.5f
			: draw.horizontalAlignment == UIHorizontalAlignment::Right ? 1.0f : 0.0f;
		const float vertical = draw.verticalAlignment == UIVerticalAlignment::Middle ? 0.5f
			: draw.verticalAlignment == UIVerticalAlignment::Bottom ? 1.0f : 0.0f;
		const glm::vec2 position = rect.pixelTopLeft + (rect.pixelSize - extent) * glm::vec2(horizontal, vertical);
		_drawText(draw.font, *font, draw.text, position, draw.color, scale);
	}

	void TextRenderer::render(RenderContext& context)
	{
		if (!m_VAO || !context.output || (context.frame.textDraws.empty() && context.frame.uiDraws.empty()))
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
		glBindVertexArray(m_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

		const glm::vec2 outputSize(context.output->getWidth(), context.output->getHeight());
		m_shader.setVec2("outputSize", outputSize);
		for (const auto& draw : context.frame.uiDraws)
		{
			if (const auto* rect = std::get_if<UIRectDraw>(&draw))
				_drawRectangle(*rect, outputSize, context);
			else
				_drawUIText(std::get<UITextDraw>(draw), outputSize, context.resources);
		}
		for (const auto& draw : context.frame.textDraws)
			if (const auto* font = context.resources.getResource<FontData>(draw.font))
				_drawText(draw.font, *font, draw.text, draw.position, draw.color, draw.scale);

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
