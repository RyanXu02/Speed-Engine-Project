#pragma once

#if defined(COMPILE_WITH_EDITOR)

#include "../../../Features/MemoryTrackingSystem.h"
#include "../Widget.h"

namespace SE
{
	class MemoryWidget : public VisibleWidget<MemoryWidget, false>
	{
	public:
		MemoryWidget()
			: VisibleWidget("Memory Panel")
		{}

		void render() override;
		void shutdown() override;
	};

} // namespace SE

#endif
