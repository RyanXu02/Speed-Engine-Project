#pragma once
#include "../Widget.h"

namespace SE
{
    class TestWidget : public Widget
    {
    public:
		TestWidget() : Widget("TestWidget") {};
		void render() override;
		void shutdown() override;
    private:

    };
}

