#pragma once
#include "Core/ResourceManagers/Shader/Shader.h"
#include "Core/SubSystems/Graphics/Rendering/Renderer.h"

namespace SE
{
	// @brief Deferred lighting pass, shades the G-buffer into the output and copies the G-buffer depth over
	// so passes reading the output depth test against the opaque geometry
	class DeferredLightingRenderer : public Renderer
	{
	public:
		// @param gBufferTarget the name of the G-buffer render target written by the GeometryRenderer
		DeferredLightingRenderer(Logger& logger, std::string gBufferTarget);

		void init() override;
		void render(RenderContext& context) override;
		void shutdown() override;

	private:
		std::string m_gBufferTarget;
		Shader m_shader;
		unsigned int m_VAO = 0;
	};
} // namespace SE
