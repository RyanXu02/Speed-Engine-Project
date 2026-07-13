#pragma once
#include "../../../Logger/LoggerProxy.h"

namespace SE
{
    class Renderer
    {
    public:
		Renderer(Logger& logger) : m_logger(logger, fmt::format("Renderer: Unnamed")) {}
        virtual ~Renderer() {};

        virtual bool init() = 0;
        virtual bool shutdown() = 0;
        virtual void update() = 0;

        virtual void beginFrame() = 0;
        virtual void endFrame() = 0;
        virtual void render() = 0;

    private:
		LoggerProxy m_logger;
    };
}

