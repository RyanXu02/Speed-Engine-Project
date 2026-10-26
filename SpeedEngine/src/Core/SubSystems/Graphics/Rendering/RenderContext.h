#pragma once
#include "FBO.h"
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

	// @brief Borrows the completed frame and the manager's rendering resources.
	struct RenderContext
	{
		const RenderFrame& frame;
		Viewport& viewport;
		ResourceManager& resources;
		MeshGPUCache& meshes;
		const RenderTargets& targets;
		FBO* output = nullptr;

		const FBO& getRenderTarget(const std::string& name) const
		{
			return *targets.at(name).framebuffer;
		}
	};
} // namespace SE
