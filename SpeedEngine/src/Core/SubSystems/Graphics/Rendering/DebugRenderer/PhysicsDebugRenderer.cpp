#include "pch.h"

#include "PhysicsDebugRenderer.h"

namespace SE
{
	void PhysicsDebugRenderer::DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor)
	{
		const glm::vec3 from(inFrom.GetX(), inFrom.GetY(), inFrom.GetZ());
		const glm::vec3 to(inTo.GetX(), inTo.GetY(), inTo.GetZ());
		const glm::vec4 color(inColor.r, inColor.g, inColor.b, inColor.a);
		m_debugRenderer.drawLine(from, to, color);
	}

	void PhysicsDebugRenderer::DrawText3D(JPH::RVec3Arg inPosition, const std::string_view& inString, JPH::ColorArg inColor, float inHeight)
	{
		// No implementation for now
		return;
	}
}
