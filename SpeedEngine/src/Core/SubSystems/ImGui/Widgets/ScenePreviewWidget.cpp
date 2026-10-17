#include "pch.h"

#if defined(COMPILE_WITH_EDITOR)
#include "../../Graphics/Rendering/RendererManager.h"
#include "Engine.h"
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

		ImGuiWindowFlags window_flags = 0; // Default flags allow docking
		if (!ImGui::Begin("ScenePreviewWidget", &s_visible, window_flags))
		{
			ImGui::End();
			return;
		}

		// This widget (along with other imgui widgets) receive the imgui viewport,
		// but we want to display the scene viewport in this widget
		uint32_t fboColorId = Engine::Instance().getSubSystem<RendererManager>()->getRenderer<SceneRenderer>()->getViewport()->getFBO().getColorTexture();

		ImTextureID imguiTexId = static_cast<ImTextureID>(fboColorId);
		ImVec2 viewportSize = ImGui::GetContentRegionAvail();

		ImGui::Image(fboColorId, viewportSize, ImVec2(0, 1), ImVec2(1, 0));

		ImGui::End();
	}

	void ScenePreviewWidget::shutdown()
	{
		_setStatus("Shutting down ScenePreviewWidget");
	}
} // namespace SE
#endif
