#pragma once
#include "../../../Logger/LoggerProxy.h"

#include "Viewport.h"

namespace SE
{
    class Viewport;

	// @brief Abstract base class for all renderers in the engine
    class Renderer
    {
    public:
		Renderer(Logger& logger, std::string_view rendererName) : m_logger(logger, fmt::format("Renderer: {}", rendererName)) {}
		Renderer(Logger& logger, std::string_view rendererName, std::unique_ptr<Viewport> viewport) : m_logger(logger, fmt::format("Renderer: {}", rendererName)), m_viewport(std::move(viewport)) {}
        virtual ~Renderer() {};

        virtual void init() = 0;
        virtual void shutdown() = 0;
        virtual void update(double deltaTime) = 0;
        virtual void render() const = 0;

		Viewport* getViewport() const {
			if (m_viewport)
				return m_viewport.get();
			else
				return nullptr;
		}

    protected:
		LoggerProxy m_logger;
		std::unique_ptr<Viewport> m_viewport{ nullptr };
    };
}

