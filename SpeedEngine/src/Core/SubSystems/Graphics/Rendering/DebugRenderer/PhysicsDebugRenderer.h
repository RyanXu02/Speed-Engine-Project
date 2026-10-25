#pragma once

#include "DebugRenderer.h"

#include <Jolt/Renderer/DebugRendererSimple.h>

namespace SE
{
	class PhysicsDebugRenderer : public JPH::DebugRendererSimple
	{
	public:
		PhysicsDebugRenderer(SE::DebugRenderer& debugRenderer)
		    : m_debugRenderer(debugRenderer) {};

		virtual void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override;

		virtual void DrawText3D(JPH::RVec3Arg inPosition, const std::string_view& inString, JPH::ColorArg inColor, float inHeight) override;

	private:
		SE::DebugRenderer& m_debugRenderer;
	};
}
