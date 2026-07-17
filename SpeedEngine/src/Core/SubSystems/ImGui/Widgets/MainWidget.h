#pragma once
#include "../Widget.h"

namespace SE
{
    class MainWidget : public Widget
    {
    public:
		MainWidget() : Widget("MainWidget") {};

        void render() override;
		void shutdown() override;
    private:

    };
}

