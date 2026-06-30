#pragma once
#include "../SubSystems/SubSystem.h"
#include "Resource.h"

namespace SE
{
    class Manager : public SubSystem
    {
    public:
		Manager(ResourceType rt, const std::string& name) : SubSystem(name), m_resourceType(rt) {}
        
        virtual void init() override { SubSystem::init(); }
        virtual void shutdown() override { SubSystem::shutdown(); }
    protected:
		ResourceType m_resourceType;
    private:

    };
}

