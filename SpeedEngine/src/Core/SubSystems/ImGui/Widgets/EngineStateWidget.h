#pragma once

#if defined(COMPILE_WITH_EDITOR)

#include "../Widget.h"

namespace SE
{
	class EngineStateWidget : public VisibleWidget<EngineStateWidget>
	{
	public:
		EngineStateWidget()
			: VisibleWidget("Engine State")
		{}

		void render() override;
		void shutdown() override;
	};

} // namespace SE

#endif
