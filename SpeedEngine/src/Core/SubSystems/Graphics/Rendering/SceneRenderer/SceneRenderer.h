#pragma once
#include "../Renderer.h"

namespace SE
{
    class SceneRenderer : public Renderer
    {
    public:
		SceneRenderer(RenderContext& context, Logger& logger) : Renderer(context, logger, "SceneRenderer") {};

        bool init() override;
        void shutdown() override;
        void update(double deltaTime) override;

        void beginFrame() override;
        void endFrame() override;
        void render() const override;

    private:
    };
}

