#include "pch.h"

#if defined(COMPILE_WITH_EDITOR)
#include "Core/SubSystems/Graphics/Rendering/RendererManager.h"
#include "Engine.h"
#include "EngineStateWidget.h"
#include "ScenePreviewWidget.h"

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

		constexpr float previewVerticalPadding = 2.0f;
		constexpr float toolbarImageSpacing = 2.0f;
		const ImGuiStyle& style = ImGui::GetStyle();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(style.WindowPadding.x, previewVerticalPadding));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x, toolbarImageSpacing));
		ImGuiWindowFlags window_flags = 0; // Default flags allow docking
		if (!ImGui::Begin("ScenePreviewWidget", &s_visible, window_flags))
		{
			ImGui::End();
			ImGui::PopStyleVar(2);
			return;
		}

		EngineStateWidget::render();

		auto* rendererManager = Engine::Instance().getSubSystem<RendererManager>();
		ImTextureID imguiTexId = static_cast<ImTextureID>(rendererManager->getFinalSceneTexture());
		ImVec2 viewportSize = ImGui::GetContentRegionAvail();

		if (viewportSize.x > 0.0f && viewportSize.y > 0.0f)
		{
			ImVec2 framebufferScale = ImGui::GetIO().DisplayFramebufferScale;
			rendererManager->requestSceneResize(static_cast<uint32_t>(viewportSize.x * framebufferScale.x),
				static_cast<uint32_t>(viewportSize.y * framebufferScale.y));
			// Weird ImGui mouse conflict with scene preview
			// Consume viewport clicks so ImGui don't start moving the preview window
			ImGui::InvisibleButton("##SceneInput", viewportSize);

			const ImVec2 imageMin = ImGui::GetItemRectMin();
			const ImVec2 imageMax = ImGui::GetItemRectMax();
			ImGui::GetWindowDrawList()->AddImage(imguiTexId, imageMin, imageMax, ImVec2(0, 1), ImVec2(1, 0));
			const ImVec2 windowOrigin = ImGui::GetMainViewport()->Pos;

			rendererManager->setGameInputRegion({
				{ imageMin.x - windowOrigin.x, imageMin.y - windowOrigin.y },
				{ imageMax.x - imageMin.x, imageMax.y - imageMin.y }, rendererManager->getFinalSceneSize(),
				ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) }, ImGui::IsItemActive());
		}

		ImGui::End();
		ImGui::PopStyleVar(2);
	}

	void ScenePreviewWidget::shutdown()
	{
		_setStatus("Shutting down ScenePreviewWidget");
	}
} // namespace SE
#endif
