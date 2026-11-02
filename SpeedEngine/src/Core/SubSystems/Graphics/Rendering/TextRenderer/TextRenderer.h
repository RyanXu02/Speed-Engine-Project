#pragma once

#include "Core/ResourceManagers/Font/Font.h"
#include "Core/ResourceManagers/Shader/Shader.h"
#include "Core/SubSystems/Graphics/Rendering/Renderer.h"

namespace SE
{
	class TextRenderer : public Renderer
	{
	public:
		explicit TextRenderer(Logger& logger)
			: Renderer(logger, "TextRenderer"), m_shader(0, "TextRenderer")
		{}

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

		Shader m_shader;
		unsigned int m_VAO = 0;
		unsigned int m_VBO = 0;
		std::unordered_map<FontHandle, unsigned int> m_atlases;
		std::vector<Vertex> m_vertices;
	};
} // namespace SE
