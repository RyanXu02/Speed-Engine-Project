#include "pch.h"

#include "PhysicsDebugRenderer.h"
#include "Core/SubSystems/Graphics/Rendering/RenderFrame.h"

#include <Jolt/Physics/PhysicsSystem.h>
#include <cassert>

namespace SE
{
	void PhysicsDebugRenderer::appendGeometry(RenderFrame& frame, JPH::PhysicsSystem& physics, const glm::vec3& cameraPosition)
	{
		assert(!m_frame);
		m_frame = &frame;
		struct ResetOutput
		{
			RenderFrame*& output;
			~ResetOutput() { output = nullptr; }
		} reset{ m_frame };

		SetCameraPos(JPH::RVec3(cameraPosition.x, cameraPosition.y, cameraPosition.z));
		JPH::BodyManager::DrawSettings settings;
		settings.mDrawShape = true;
		settings.mDrawShapeWireframe = true;
		settings.mDrawVelocity = true;
		settings.mDrawGetSupportingFace = true;
		physics.DrawBodies(settings, this);
	}

	void PhysicsDebugRenderer::DrawLine(JPH::RVec3Arg from, JPH::RVec3Arg to, JPH::ColorArg color)
	{
		if (m_frame)
			m_frame->addDebugLine(
				glm::vec3(from.GetX(), from.GetY(), from.GetZ()),
				glm::vec3(to.GetX(), to.GetY(), to.GetZ()),
				glm::vec4(color.r, color.g, color.b, color.a)/255.0f);
	}

	void PhysicsDebugRenderer::DrawTriangle(JPH::RVec3Arg a, JPH::RVec3Arg b, JPH::RVec3Arg c, JPH::ColorArg color, ECastShadow)
	{
		if (m_frame)
			m_frame->addDebugTriangle(
				glm::vec3(a.GetX(), a.GetY(), a.GetZ()),
				glm::vec3(b.GetX(), b.GetY(), b.GetZ()),
				glm::vec3(c.GetX(), c.GetY(), c.GetZ()),
				glm::vec4(color.r, color.g, color.b, color.a)/255.0f);
	}

	void PhysicsDebugRenderer::DrawText3D(JPH::RVec3Arg, const std::string_view&, JPH::ColorArg, float)
	{
		// No implementation for now
	}
} // namespace SE
