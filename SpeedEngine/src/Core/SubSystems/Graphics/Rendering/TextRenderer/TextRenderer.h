#pragma once

#include "Core/ResourceManagers/Font/Font.h"
#include "Core/ResourceManagers/Shader/Shader.h"
#include "Core/SubSystems/Graphics/Rendering/Renderer.h"

namespace SE
{
	struct UIRectDraw;
	struct UITextDraw;
	class ResourceManager;

	class TextRenderer : public Renderer
	{
	public:
		explicit TextRenderer(Logger& logger)
			: Renderer(logger, "TextRenderer"), m_shader(0, "TextRenderer")
		{}
		~TextRenderer() override { shutdown(); }

		void init() override;
		void shutdown() override;
		void render(RenderContext& context) override;

	private:
		struct Vertex
		{
			glm::vec2 position;
			glm::vec2 uv;
		};

		// @brief Uploads the font atlas to the GPU if it hasn't been uploaded yet, and returns the OpenGL texture ID
		// @param font The font handle
		// @return The OpenGL texture ID of the font atlas
		unsigned int _getOrUpload(const FontHandle& font);

		void _appendQuad(glm::vec2 a, glm::vec2 b, glm::vec2 uvMin, glm::vec2 uvMax);

		void _drawVertices(unsigned int texture, const glm::vec4& color, int mode);
		void _drawText(const FontHandle& font, std::string_view text, glm::vec2 position, glm::vec4 color, float scale);
		void _drawRectangle(const UIRectDraw& draw, glm::vec2 outputSize, ResourceManager& resources);
		void _drawUIText(const UITextDraw& draw, glm::vec2 outputSize, ResourceManager& resources);

		Shader m_shader;
		unsigned int m_VAO = 0;
		unsigned int m_VBO = 0;

		std::unordered_map<FontHandle, unsigned int> m_atlases;
		std::vector<Vertex> m_vertices;
	};
} // namespace SE
