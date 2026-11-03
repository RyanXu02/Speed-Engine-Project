#pragma once
#include "Core/SubSystems/Graphics/Rendering/Renderer.h"
#include "Core/ResourceManagers/Shader/Shader.h"

namespace SE
{
	// @brief Draws CPU-simulated billboard particles from RenderFrame::particles
	// @brief Draws on top of an existing target; depth tested, no depth writes, alpha blended.
	// @brief will eventually get in geometric (3d) particles
	class ParticleRenderer : public Renderer
	{
	public:
		explicit ParticleRenderer(Logger& logger)
		    : Renderer(logger, "ParticleRenderer"),
		      m_bbShader(0, "ParticleRenderer")
		{}

		void init() override;
		void shutdown() override;
		void render(RenderContext& context) override;

	private:
		unsigned int m_VAO = 0;
		unsigned int m_quadVBO = 0;
		unsigned int m_instanceVBO = 0;
		Shader m_bbShader;
	};
} // namespace SE
