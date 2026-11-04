#pragma once
#include "Core/ResourceManagers/Shader/Shader.h"
#include "Core/SubSystems/Graphics/Rendering/Renderer.h"

namespace SE
{
	enum class TonemapCurve : int
	{
		None,
		Reinhard,
		ACES
	};

	// @brief Final pass, maps the HDR scene color to display range, gamma corrects and dithers it into the output
	class TonemapRenderer : public Renderer
	{
	public:
		// @param inputTarget the name of the HDR render target to read from
		TonemapRenderer(Logger& logger, std::string inputTarget);

		void init() override;
		void render(RenderContext& context) override;
		void shutdown() override;

		TonemapCurve curve = TonemapCurve::ACES;
		float exposure = 1.0f;
		float gamma = 2.2f;
		bool dither = true;		// adds sub step noise before the 8 bit output to break up banding

	private:
		std::string m_inputTarget;
		Shader m_shader;
		unsigned int m_VAO = 0;
	};
} // namespace SE
