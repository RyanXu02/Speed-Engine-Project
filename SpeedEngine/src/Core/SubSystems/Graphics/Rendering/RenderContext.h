#pragma once
#include "FBO.h"
#include "AssetGPUCache.h"
#include "MeshGPUCache.h"
#include "RenderFrame.h"
#include "Viewport.h"

namespace SE
{
	class ResourceManager;

	// @brief Describes the size of a render target, either following the scene viewport or using a fixed scale
	struct RenderTargetSize
	{
		bool followSceneViewport = true;
		float scale = 1.0f;
	};

	struct RenderTarget
	{
		std::unique_ptr<FBO> framebuffer;
		RenderTargetSize size;
	};

	using RenderTargets = std::unordered_map<std::string, RenderTarget>;

	// only the sun is a directional light
	struct DirectionalLight
	{
		bool enabled = true;
		glm::vec3 direction{ -0.3f, -1.0f, -0.5f }; // the way the light travels, from the sun toward the ground
		glm::vec3 color{ 1.0f };
		float intensity = 3.0f;
	};

	// @brief Borrows the completed frame and the manager's rendering resources.
	struct RenderContext
	{
		const RenderFrame& frame;
		Viewport& viewport;
		ResourceManager& resources;
		MeshGPUCache& meshes;
		AssetGPUCache& gpu;
		const RenderTargets& targets;
		const DirectionalLight& sun;
		float t = 1.0f; // for interpolation
		FBO* output = nullptr;

		const FBO& getRenderTarget(const std::string& name) const
		{
			return *targets.at(name).framebuffer;
		}
	};
} // namespace SE
