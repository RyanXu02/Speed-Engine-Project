#include "pch.h"

#include "RenderFrameBuilder.h"
#include "RenderFrame.h"

#include "Core/EngineStates.h"
#include "Core/Scene/Scene.h"
#include "Core/SubSystems/Physics/PhysicsSystem.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

namespace SE
{
	void RenderFrameBuilder::captureSimulation(Scene& scene, std::chrono::steady_clock::time_point tickTime)
	{
		++m_tick;
		if (&scene != m_lastScene)
			m_drawItemHistories.clear();

		scene.forEntitiesWith<Transform, NameComponent, MeshComponent>([&](Entity entity)
		{
			const auto meshId = entity.getComponent<MeshComponent>()->meshResourceID;
			if (meshId == 0)
				return;

			const Transform& transform = *entity.getComponent<Transform>();
			const TransformState transformState{ transform.getTranslation(), transform.getRotationQuat(), transform.getScale() };

			auto [it,_] = m_drawItemHistories.try_emplace(entity.id(), 
				DrawItemHistory{ { meshId, entity.id(), transformState, transformState }, m_tick });
			auto& entry = it->second;
			entry.drawItem.previous = entry.drawItem.current;
			entry.drawItem.current = transformState;
			entry.drawItem.meshResourceId = meshId;
			entry.lastSeenTick = m_tick;
		});
		// Remove entries that were not present in this tick
		std::erase_if(m_drawItemHistories, [this](const auto& entry) { return entry.second.lastSeenTick != m_tick; });

		// capture camera state
		const CameraFrustum* activeCamera = EngineStates::getActiveCamera();
		const CameraState camera = activeCamera->getState();
		m_camera.previous = activeCamera != m_lastCamera ? camera : m_camera.current;
		m_camera.current = camera;
		m_lastScene = &scene;
		m_lastCamera = activeCamera;
		m_tickTime = tickTime;
	}

	void RenderFrameBuilder::build(RenderFrame& frame, PhysicsSystem& physics) const
	{
		frame.draws.clear();
		frame.debugLines.clear();
		frame.debugTriangles.clear();

		frame.draws.reserve(m_drawItemHistories.size());
		for (const auto& [id, entry] : m_drawItemHistories)
			frame.draws.push_back(entry.drawItem);

		frame.camera = m_camera;
		frame.tickTime = m_tickTime;
		physics.appendDebugGeometry(frame, frame.camera.current.position);
	}
} // namespace SE
