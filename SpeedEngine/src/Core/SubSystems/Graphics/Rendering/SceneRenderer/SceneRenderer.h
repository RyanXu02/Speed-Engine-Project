#pragma once
#include "../Renderer.h"

namespace SE
{
    class SceneRenderer : public Renderer
    {
    public:
		SceneRenderer(Logger& logger) : Renderer(logger, "SceneRenderer") {};

        bool init() override;
        void shutdown() override;
        void update(double deltaTime) override;

        void render(Viewport& viewport) const override;

    private:
    };
}

