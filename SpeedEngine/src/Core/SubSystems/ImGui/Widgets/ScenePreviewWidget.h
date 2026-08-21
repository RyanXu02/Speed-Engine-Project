#pragma once
#include "../Widget.h"
namespace SE 
{
	class ScenePreviewWidget : public Widget
	{
	public:
		ScenePreviewWidget();

		void render();
		void shutdown() override;
	private:
	};
}

