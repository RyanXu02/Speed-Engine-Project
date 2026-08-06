#pragma once
#include "../Renderer.h"

namespace SE
{
    class Entity;
    struct DrawData;

    // GPU mesh data cache
    struct MeshGPUData {
        unsigned int VAO = 0;
        unsigned int VBO = 0;
        unsigned int EBO = 0;
        size_t indexCount = 0;
        bool needsUpdate = true;
    };

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

        // Helper methods for mesh rendering
        void uploadMeshToGPU(uint32_t meshId, const DrawData& drawData) const;
        void cleanupMeshGPU(uint32_t meshId) const;

        // Cache of GPU mesh data (mutable for const render method)
        mutable std::unordered_map<uint32_t, MeshGPUData> m_meshCache;

        // Default shader for meshes without material
        uint32_t m_defaultShaderId = 1;
    };
}

