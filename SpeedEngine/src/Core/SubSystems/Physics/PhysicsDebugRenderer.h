#pragma once

#include <Jolt/Renderer/DebugRendererSimple.h>
#include <glm/glm.hpp>

namespace JPH
{
	class PhysicsSystem;
}

namespace SE
{
	struct RenderFrame;

	class PhysicsDebugRenderer : public JPH::DebugRendererSimple
	{
	public:
		void appendGeometry(RenderFrame& frame, JPH::PhysicsSystem& physics, const glm::vec3& cameraPosition);

		void DrawLine(JPH::RVec3Arg from, JPH::RVec3Arg to, JPH::ColorArg color) override;
		void DrawTriangle(JPH::RVec3Arg a, JPH::RVec3Arg b, JPH::RVec3Arg c, JPH::ColorArg color, ECastShadow castShadow) override;
		void DrawText3D(JPH::RVec3Arg position, const std::string_view& text, JPH::ColorArg color, float height) override;

	private:
		RenderFrame* m_frame = nullptr;
		std::size_t m_debugLineCapacity = 0;
		std::size_t m_debugTriangleCapacity = 0;
	};
} // namespace SE
