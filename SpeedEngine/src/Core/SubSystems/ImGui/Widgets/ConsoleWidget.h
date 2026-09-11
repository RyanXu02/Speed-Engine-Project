#pragma once
#include "../Widget.h"

namespace SE
{
    class ConsoleWidget : public Widget
    {
    public:
        ConsoleWidget() : Widget("ConsoleWidget") {};
		void render() override;
		void shutdown() override;
    private:

    };
}

