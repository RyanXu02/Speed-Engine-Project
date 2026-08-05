#pragma once
#include "../Renderer.h"

namespace SE
{
    class Entity;

    class SceneRenderer : public Renderer
    {
    public:
		SceneRenderer(Logger& logger) : Renderer(logger, "SceneRenderer") {};

        void init() override;
        void shutdown() override;
        void update(double deltaTime) override;

        void render(Viewport& viewport) const override;


    private:
        std::vector<Entity*> getDrawableEntities() const;
    };
}

