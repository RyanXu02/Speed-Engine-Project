#include "pch.h"

#include "UISystem.h"
#include "Engine.h"
#include "SubSystems/Graphics/Window.h"
#include "SubSystems/Input/InputSystem.h"
#include "SubSystems/Scenes/SceneSystem.h"

#include <GLFW/glfw3.h>
#include <imgui.h>

namespace SE
{
	void UISystem::captureInput(glm::uvec2 renderTargetSize)
	{
		auto* window = Engine::Instance().getSubSystem<Window>()->getWindow();
		auto region = m_sceneInputRegion;
		m_sceneInputRegion = {};

#if !defined(COMPILE_WITH_EDITOR)
		int windowWidth = 0, windowHeight = 0;
		glfwGetWindowSize(window, &windowWidth, &windowHeight);
		region = { { 0.0f, 0.0f }, { windowWidth, windowHeight }, true };
#endif

		int framebufferWidth = 0, framebufferHeight = 0;
		glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
		bool blocked = ImGui::IsAnyItemActive() ||
			ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel);

#if !defined(COMPILE_WITH_EDITOR)
		blocked |= ImGui::GetIO().WantCaptureMouse;
#endif

		InputSnapshot snapshot{ glm::vec2(renderTargetSize), std::nullopt };
		if (region.pointerEnabled && !blocked && glfwGetWindowAttrib(window, GLFW_FOCUSED) &&
			framebufferWidth > 0 && framebufferHeight > 0 &&
			region.windowSize.x > 0.0f && region.windowSize.y > 0.0f &&
			renderTargetSize.x > 0 && renderTargetSize.y > 0)
		{
			double x = 0.0, y = 0.0;
			glfwGetCursorPos(window, &x, &y);
			const glm::vec2 local = glm::vec2(static_cast<float>(x), static_cast<float>(y)) - region.windowTopLeft;
			if (local.x >= 0.0f && local.y >= 0.0f && local.x < region.windowSize.x && local.y < region.windowSize.y)
				snapshot.pointerPosition = local / region.windowSize * snapshot.renderTargetSize;
		}

		std::lock_guard lock(m_inputMutex);
		m_pendingInput = snapshot;
	}

	void UISystem::update(double deltaTime)
	{
		SubSystem::update(deltaTime);
		m_resolvedElements.clear();
		{
			std::lock_guard lock(m_inputMutex);
			if (!Engine::Instance().getSubSystem<InputSystem>()->hasFocus())
				m_pendingInput.pointerPosition.reset();
			m_input = m_pendingInput;
		}
		const glm::vec2 renderTargetSize = getRenderTargetSize();

		auto* scene = Engine::Instance().getSubSystem<SceneSystem>()->getCurrentScene();
		if (!scene || renderTargetSize.x <= 0.0f || renderTargetSize.y <= 0.0f)
			return;

		auto elements = scene->getEntitiesWith<UIElementComponent, UIRectComponent>();
		scene->forEntitiesWith<UICanvasComponent>([&](Entity canvasEntity)
		{
			const auto& canvas = *canvasEntity.getComponent<UICanvasComponent>();
			if (!canvas.visible || canvas.canvasKey.empty() ||
				canvas.referenceResolution.x <= 0.0f || canvas.referenceResolution.y <= 0.0f)
				return;

			const auto layout = resolveUICanvasLayout(renderTargetSize, canvas.referenceResolution);
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
		m_sceneInputRegion = {};
		m_input = {};
		{
			std::lock_guard lock(m_inputMutex);
			m_pendingInput = {};
		}
		SubSystem::shutdown();
	}
}
