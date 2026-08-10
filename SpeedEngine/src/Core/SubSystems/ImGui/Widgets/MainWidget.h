#pragma once
#include "../Widget.h"

namespace SE
{
    // @brief The main widget that all other widgets are docked into
    class MainWidget : public Widget
    {
    public:
		MainWidget() : Widget("MainWidget") {};

        void render() override;
		void shutdown() override;
    private:

    };
}

