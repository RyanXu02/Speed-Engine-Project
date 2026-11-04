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
	namespace // Helper to append all UI elements in the scene to RenderFrame
	{
		static void appendUIDraws(Scene & scene, RenderFrame & frame)
		{
			std::unordered_map<std::string, glm::vec2> canvases;
			scene.forEntitiesWith<UICanvasComponent>(
			    [&](Entity entity)
			    {
				    const auto& canvas = *entity.getComponent<UICanvasComponent>();
				    if (canvas.visible && !canvas.canvasKey.empty() && canvas.referenceResolution.x > 0.0f && canvas.referenceResolution.y > 0.0f)
					    canvases.emplace(canvas.canvasKey, canvas.referenceResolution);
			    });

			struct Member
			{
				Entity entity;
				glm::vec2 referenceResolution;
				int drawOrder;
			};
			std::vector<Member> members;
			scene.forEntitiesWith<UIElementComponent, UIRectComponent>(
			    [&](Entity entity)
			    {
				    const auto& element = *entity.getComponent<UIElementComponent>();
				    const auto& rect = *entity.getComponent<UIRectComponent>();
				    const auto canvas = canvases.find(element.canvasKey);
				    if (element.visible && canvas != canvases.end() && rect.size.x > 0.0f && rect.size.y > 0.0f)
					    members.push_back({ entity, canvas->second, element.drawOrder });
			    });
			std::ranges::sort(members,
			    [](const Member& a, const Member& b)
			    {
				    if (a.drawOrder != b.drawOrder)
					    return a.drawOrder < b.drawOrder;
				    return a.entity.id() < b.entity.id();
			    });

			auto* resource = frame.uiDraws.get_allocator().resource();
			for (auto& member : members)
			{
				const auto& rect = *member.entity.getComponent<UIRectComponent>();
				glm::vec4 tint{ 1.0f };
				if (const auto* selectable = member.entity.getComponent<UISelectableComponent>())
					tint = !member.entity.getComponent<UIElementComponent>()->enabled ? selectable->disabledColor : selectable->focused ? selectable->focusedColor : selectable->normalColor;
				const auto* image = member.entity.getComponent<UIImageComponent>();
				const bool hasBackground = image && image->color.a > 0.0f;
				if (hasBackground)
				{
					frame.uiDraws.emplace_back(std::in_place_type<UIRectDraw>, rect, member.referenceResolution, *image, resource);
					std::get<UIRectDraw>(frame.uiDraws.back()).color *= tint;
				}
				if (const auto* text = member.entity.getComponent<UITextComponent>(); text && !text->text.empty() && !text->fontPath.empty() && text->fontPixelHeight > 0 && text->fontSize > 0.0f && text->color.a > 0.0f)
				{
					frame.uiDraws.emplace_back(std::in_place_type<UITextDraw>, rect, member.referenceResolution, *text, resource);
					if (!hasBackground)
						std::get<UITextDraw>(frame.uiDraws.back()).color *= tint;
				}
			}
		}
	}

	void RenderFrameBuilder::submitText(FontHandle font, std::string_view text, glm::vec2 position, glm::vec4 color)
	{
		if (font && !text.empty())
			m_textSubmissions.push_back({ std::move(font), std::string(text), position, color });
	}

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

		// capture lights, not interpolated
		m_lights.clear();
		scene.forEntitiesWith<Transform, LightComponent>([&](Entity entity)
		{
			const LightComponent* light = entity.getComponent<LightComponent>();
			m_lights.push_back({ entity.getComponent<Transform>()->getTranslation(), light->color * light->intensity });
		});

		// capture camera state
		const CameraFrustum* activeCamera = EngineStates::getActiveCamera();
		const CameraState camera = activeCamera->getState();
		m_camera.previous = activeCamera != m_lastCamera ? camera : m_camera.current;
		m_camera.current = camera;
		m_lastScene = &scene;
		m_lastCamera = activeCamera;
		m_tickTime = tickTime;
	}

	void RenderFrameBuilder::build(RenderFrame& frame, PhysicsSystem& physics, Scene& scene) const
	{
		frame.draws.clear();
		frame.lights.clear();
		frame.debugLines.clear();
		frame.debugTriangles.clear();
		frame.textDraws.clear();
		frame.uiDraws.clear();

		frame.textDraws.reserve(m_textSubmissions.size());
		for (const auto& submission : m_textSubmissions)
			frame.textDraws.emplace_back(submission.font, submission.text, submission.position,
				submission.color, frame.textDraws.get_allocator().resource());

		frame.draws.reserve(m_drawItemHistories.size());
		for (const auto& [id, entry] : m_drawItemHistories)
			frame.draws.push_back(entry.drawItem);

		frame.lights.assign(m_lights.begin(), m_lights.end());

		frame.camera = m_camera;
		frame.tickTime = m_tickTime;
		if (EngineStates::isDebugRenderingEnabled())
			physics.appendDebugGeometry(frame, frame.camera.current.position);

		appendUIDraws(scene, frame);
	}
} // namespace SE
