#pragma once
#include "Core/ResourceManagers/Shader/Shader.h"
#include "Core/SubSystems/Graphics/Rendering/RenderFrame.h"
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

		// Defer GPU allocation until the first enabled debug draw
		void init() override {}
		void shutdown() override;
		void render(RenderContext& context) override;

	private:
		void _lazyInit();
		void _draw(std::span<const DebugVertex> vertices, unsigned int mode) const;

		unsigned int m_VAO = 0;
		unsigned int m_VBO = 0;
		Shader m_shader;
	};
} // namespace SE
