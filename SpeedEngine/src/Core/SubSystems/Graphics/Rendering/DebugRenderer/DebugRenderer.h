#pragma once
#include "Core/ResourceManagers/Shader/Shader.h"
#include "Core/SubSystems/Graphics/Rendering/Renderer.h"

namespace SE
{
	// @brief Draws debug primitives for one frame
	class DebugRenderer : public Renderer
	{
	public:
		explicit DebugRenderer(Logger& logger)
			: Renderer(logger, "DebugRenderer"), m_shader(0, "DebugRenderer")
		{}

		void init() override;
		void shutdown() override;
		void render(RenderContext& context) const override;

		void drawLine(const glm::vec3& from, const glm::vec3& to, const glm::vec4& color);
		// Not sure what castShadow is for?...
		void drawTri(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, const glm::vec4& color, bool castShadow = false);

	private:
		struct DebugVertex
		{
			glm::vec3 position;
			glm::vec4 color;
		};

		void _draw(std::span<const DebugVertex> vertices, unsigned int mode) const;

		unsigned int m_VAO = 0;
		unsigned int m_VBO = 0;
		mutable Shader m_shader;
		// These are consumed each frame
		mutable std::vector<DebugVertex> m_lines;
		mutable std::vector<DebugVertex> m_triangles;
	};
} // namespace SE
