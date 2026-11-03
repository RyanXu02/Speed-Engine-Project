#include "pch.h"

#if defined(COMPILE_WITH_EDITOR)
#include "../../Graphics/Rendering/RendererManager.h"
#include "Engine.h"
#include "EngineStates.h"
#include "MainWidget.h"
#include "ScenePreviewWidget.h"
#include "SubSystems/UI/UISystem.h"

#include <imgui.h>

namespace SE
{
	ScenePreviewWidget::ScenePreviewWidget()
		: VisibleWidget("ScenePreviewWidget")
	{}

	void ScenePreviewWidget::render()
	{
		if (!s_visible) return;

		_setStatus("Rendering ScenePreviewWidget");

		ImGuiWindowFlags window_flags = 0; // Default flags allow docking
		if (!ImGui::Begin("ScenePreviewWidget", &s_visible, window_flags))
		{
			ImGui::End();
			return;
		}

		auto* rendererManager = Engine::Instance().getSubSystem<RendererManager>();
		ImTextureID imguiTexId = static_cast<ImTextureID>(rendererManager->getFinalSceneTexture());
		ImVec2 viewportSize = ImGui::GetContentRegionAvail();

		if (viewportSize.x > 0.0f && viewportSize.y > 0.0f)
		{
			ImVec2 framebufferScale = ImGui::GetIO().DisplayFramebufferScale;
			rendererManager->requestSceneResize(static_cast<uint32_t>(viewportSize.x * framebufferScale.x),
				static_cast<uint32_t>(viewportSize.y * framebufferScale.y));
			ImGui::Image(imguiTexId, viewportSize, ImVec2(0, 1), ImVec2(1, 0));

			const ImVec2 imageMin = ImGui::GetItemRectMin();
			const ImVec2 imageMax = ImGui::GetItemRectMax();
			const ImVec2 windowOrigin = ImGui::GetMainViewport()->Pos;

			const bool pointerEnabled = EngineStates::isPlaying() && !MainWidget::isSceneEditingBlocked() &&
				ImGui::IsItemHovered() && ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

			Engine::Instance().getSubSystem<UISystem>()->setSceneInputRegion(
				{ imageMin.x - windowOrigin.x, imageMin.y - windowOrigin.y },
				{ imageMax.x - imageMin.x, imageMax.y - imageMin.y }, pointerEnabled);
		}

		ImGui::End();
	}

	void ScenePreviewWidget::shutdown()
	{
		_setStatus("Shutting down ScenePreviewWidget");
	}
} // namespace SE
#endif
