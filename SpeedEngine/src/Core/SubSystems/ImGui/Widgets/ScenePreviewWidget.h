#pragma once
#include "../Widget.h"
namespace SE
{
	class ScenePreviewWidget : public VisibleWidget<ScenePreviewWidget>
	{
	public:
		ScenePreviewWidget();

		void render() override;
		void shutdown() override;

	private:
	};
} // namespace SE
