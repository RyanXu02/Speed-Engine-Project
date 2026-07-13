#pragma once
#include "../SubSystem.h"

namespace SE
{
	class Widget;

    class ImGuiSystem : public SubSystem
    {
	public:
		void init() override;
		void update(double deltaTime) override;
		void shutdown() override;

	private:
		std::unordered_map<uint32_t, Widget*> m_widgets;
    };
}

