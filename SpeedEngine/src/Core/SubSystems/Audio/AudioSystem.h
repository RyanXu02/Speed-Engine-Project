#pragma once
#include "Core/SubSystems/Subsystem.h"

namespace SE
{
	class AudioSystem : public SubSystem
	{
		friend class Engine;

	public:
		//need to explicitly declare destructor for pimpl
		~AudioSystem();

		void init() override;
		void update(double dt) override;
		void shutdown() override;

		void PlaySound(std::string_view path);
	private:
		AudioSystem();

		//pimpl idiom
		struct AudioImpl;
		std::unique_ptr<AudioImpl> m_aimpl;
	};
} //namespace SE
