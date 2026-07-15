#pragma once
#include "../../../Logger/LoggerProxy.h"

namespace SE
{
    class Viewport;

    class Renderer
    {
    public:
		Renderer(Logger& logger, std::string_view rendererName) : m_logger(logger, fmt::format("Renderer: {}", rendererName)) {}
        virtual ~Renderer() {};

        virtual bool init() = 0;
        virtual void shutdown() = 0;
        virtual void update(double deltaTime) = 0;

        virtual void render(Viewport& viewport) const = 0;

    protected:
		LoggerProxy m_logger;
    };
}

