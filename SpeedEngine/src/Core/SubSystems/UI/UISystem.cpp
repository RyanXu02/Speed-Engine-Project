#include "pch.h"

#include "UISystem.h"
#include "Engine.h"
#include "SubSystems/Scenes/SceneSystem.h"

namespace SE
{
	void UISystem::update(double deltaTime)
	{
		SubSystem::update(deltaTime);
		m_resolvedElements.clear();

		auto* scene = Engine::Instance().getSubSystem<SceneSystem>()->getCurrentScene();
		if (!scene || m_viewportSize.x <= 0.0f || m_viewportSize.y <= 0.0f)
			return;

		auto elements = scene->getEntitiesWith<UIElementComponent, UIRectComponent>();
		scene->forEntitiesWith<UICanvasComponent>([&](Entity canvasEntity)
		{
			const auto& canvas = *canvasEntity.getComponent<UICanvasComponent>();
			if (!canvas.visible || canvas.canvasKey.empty() ||
				canvas.referenceResolution.x <= 0.0f || canvas.referenceResolution.y <= 0.0f)
				return;

			const auto layout = resolveUICanvasLayout(m_viewportSize, canvas.referenceResolution);
			for (auto entity : elements)
			{
				const auto& element = *entity.getComponent<UIElementComponent>();
				if (!element.visible || element.canvasKey != canvas.canvasKey)
					continue;

				m_resolvedElements.push_back({ entity.id(), canvasEntity.id(), element.drawOrder, layout.scale,
					resolveUIRectLayout(*entity.getComponent<UIRectComponent>(), layout) });
			}
		});

		std::ranges::sort(m_resolvedElements, [](const ResolvedUIElement& a, const ResolvedUIElement& b)
		{
			if (a.drawOrder != b.drawOrder)
				return a.drawOrder < b.drawOrder;
			return a.entityID < b.entityID;
		});
	}

	void UISystem::shutdown()
	{
		m_resolvedElements.clear();
		SubSystem::shutdown();
	}
}
