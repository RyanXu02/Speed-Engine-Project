#pragma once

#if defined(COMPILE_WITH_EDITOR)

#include "../../../Features/MemoryTrackingSystem.h"
#include "../Widget.h"

namespace SE
{
	class MemoryWidget : public Widget
	{
	public:
		MemoryWidget()
		    : Widget("Memory Panel")
		{}

		void render() override;
		void shutdown() override;

		static void setVisible(bool v);
		static bool isVisible();

	private:
		// visibility controlled by menu
		static inline bool s_visible = false;
	};

} // namespace SE

#endif
