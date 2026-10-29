#include "pch.h"

#include "RenderFrameBuilder.h"
#include "RenderFrame.h"

#include "Core/EngineStates.h"
#include "Core/Scene/Scene.h"
#include "Core/SubSystems/Physics/PhysicsSystem.h"

namespace SE
{
	void RenderFrameBuilder::build(RenderFrame& frame, Scene* scene, PhysicsSystem& physics) const
	{
		frame.draws.clear();
		frame.lights.clear();
		frame.debugLines.clear();
		frame.debugTriangles.clear();

		if (scene)
		{
			auto entities = scene->getEntitiesWith<Transform, NameComponent, MeshComponent>();
			frame.draws.reserve(entities.size());
			for (auto& entity : entities)
			{
				const auto meshId = entity.getComponent<MeshComponent>()->meshResourceID;
				if (meshId != 0)
					frame.draws.push_back({ meshId, entity.id(), entity.getComponent<Transform>()->getTransform() });
			}

			auto lights = scene->getEntitiesWith<Transform, LightComponent>();
			frame.lights.reserve(lights.size());
			for (auto& entity : lights)
			{
				const LightComponent* light = entity.getComponent<LightComponent>();
				frame.lights.push_back({ entity.getComponent<Transform>()->getTranslation(), light->color * light->intensity });
			}
		}

		frame.camera = EngineStates::getActiveCamera()->getState();
		physics.appendDebugGeometry(frame, frame.camera.position);
	}
} // namespace SE
