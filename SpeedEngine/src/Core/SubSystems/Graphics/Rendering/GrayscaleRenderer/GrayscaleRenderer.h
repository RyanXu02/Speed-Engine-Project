#pragma once
#include "Core/ResourceManagers/Shader/Shader.h"
#include "Core/SubSystems/Graphics/Rendering/Renderer.h"

namespace SE
{
	// @brief Example of consuming an earlier pass's output
	class GrayscaleRenderer : public Renderer
	{
	public:
		// @prarm inputTarget the name of the render target to read from
		GrayscaleRenderer(Logger& logger, std::string inputTarget);

		void init() override;
		void render(RenderContext& context) override;
		void shutdown() override;

	private:
		std::string m_inputTarget;
		Shader m_shader;
		unsigned int m_VAO = 0;
	};
} // namespace SE
