#pragma once
#include "../Renderer.h"

#include "../../../../ResourceManagers/MeshResource/MeshResource.h"


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

	// @brief SceneRenderer is responsible for rendering all drawable entities in the scene
    class SceneRenderer : public Renderer
    {
		friend class RendererManager;
    public:

        void init() override;
        void shutdown() override;
        void update(double deltaTime) override;

        void render() const override;

        // @brief get scene preview camera sensitivity
		// @return reference to camera sensitivity, used for imgui widgets
		float& getCameraSensitivity() { return m_sensitivity; }
        // @brief get scene preview camera move speed
        // @return reference to camera move speed, used for imgui widgets
		float& getCameraMoveSpeed() { return m_cameraMoveSpeed; }
        // @brief get scene preview camera dolly speed
        // @return reference to camera dolly speed, used for imgui widgets
        float& getCameraDollySpeed() { return m_cameraDollySpeed; }

    private:
		SceneRenderer(Logger& logger, std::unique_ptr<Viewport> viewport) : Renderer(logger, "SceneRenderer", std::move(viewport)) {}
        
        std::vector<Entity*> getDrawableEntities() const;

        // Helper methods for mesh rendering
        void uploadMeshToGPU(
            uint32_t meshId, 
            const std::vector<Vertex>& positions, 
            const std::vector<SubMesh>& subMeshes, 
            const std::vector<uint32_t>& indices
        ) const;
        void cleanupMeshGPU(uint32_t meshId) const;

        // Cache of GPU mesh data (mutable for const render method)
        mutable std::unordered_map<uint32_t, MeshGPUData> m_meshCache;

        // Default shader for meshes without material
        uint32_t m_defaultShaderId = 1;

        // Camera controls
		void _updateCameraPosition(double deltaTime, CameraFrustum& camera);
		void _updateCameraTarget(double deltaTime, CameraFrustum& camera);
		void _updateCamera(double deltaTime, CameraFrustum& camera, FBO& fbo);
		float m_sensitivity = 0.005f;
		float m_cameraMoveSpeed = 6.0f;
		float m_cameraDollySpeed = 10.0f;
    };
}

