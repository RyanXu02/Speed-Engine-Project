#pragma once
#include "Core/Logger/LoggerProxy.h"

namespace SE
{
	struct RenderContext;

	// @brief Abstract base class for all renderers in the engine
	// @brief Draws using resources and an output selected by RendererManager
	class Renderer
	{
	public:
		Renderer(Logger& logger, std::string_view rendererName) 
			: m_logger(logger, fmt::format("Renderer: {}", rendererName)) 
		{}
		virtual ~Renderer(){};

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer(Renderer&&) = delete;
		Renderer& operator=(Renderer&&) = delete;

		virtual void init() = 0;
		virtual void shutdown() = 0;
		virtual void update(double deltaTime, RenderContext& context) {}
		virtual void render(RenderContext& context) = 0;

	protected:
		LoggerProxy m_logger;
	};
} // namespace SE
