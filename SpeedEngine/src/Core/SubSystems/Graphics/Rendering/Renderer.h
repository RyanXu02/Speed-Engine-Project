#pragma once
#include "../../../Logger/LoggerProxy.h"

namespace SE
{
    class RenderContext;

    class Renderer
    {
    public:
		Renderer(RenderContext& context, Logger& logger, std::string_view rendererName) : m_context(&context), m_logger(logger, fmt::format("Renderer: {}", rendererName)) {}
        virtual ~Renderer() {};

        virtual bool init() = 0;
        virtual void shutdown() = 0;
        virtual void update(double deltaTime) = 0;

        virtual void beginFrame() = 0;
        virtual void endFrame() = 0;
        virtual void render() const = 0;

    protected:
		LoggerProxy m_logger;
        RenderContext* m_context;
    };
}

