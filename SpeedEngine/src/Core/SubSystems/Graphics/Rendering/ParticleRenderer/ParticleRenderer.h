#pragma once
#include "Core/SubSystems/Graphics/Rendering/Renderer.h"
#include "Core/ResourceManagers/Shader/Shader.h"

namespace SE
{
	enum class ParticleBlendMode : int
	{
		Alpha,
		Additive
	};

	// @brief Draws CPU-simulated billboard particles from RenderFrame::particles
	// @brief will eventually get in geometric (3d) particles
	class ParticleRenderer : public Renderer
	{
	public:
		explicit ParticleRenderer(Logger& logger)
		    : Renderer(logger, "ParticleRenderer"),
			  m_bbShader()
		{}

		void init() override;
		void shutdown() override;
		void render(RenderContext& context) override;

		ParticleBlendMode blendMode = ParticleBlendMode::Alpha;

	private:
		unsigned int m_VAO = 0;
		unsigned int m_quadVBO = 0;
		unsigned int m_instanceVBO = 0;
		Shader m_bbShader;
	};
} // namespace SE
