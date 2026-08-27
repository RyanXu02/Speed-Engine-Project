#pragma once
#include "../Widget.h"
namespace SE 
{
	class ScenePreviewWidget : public Widget
	{
	public:
		ScenePreviewWidget();

		void render() override;
		void shutdown() override;
	private:
	};
}

