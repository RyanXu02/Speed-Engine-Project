#pragma once

namespace SE
{
	class Scene;
	class PhysicsSystem;
	struct RenderFrame;

	class RenderFrameBuilder
	{
	public:
		void build(RenderFrame& frame, Scene* scene, PhysicsSystem& physics) const;
	};
} // namespace SE
