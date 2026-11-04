#pragma once
#include <random>

namespace SE
{
	class RNG
	{
	public:
		inline static float Range(float rangeMin, float rangeMax) { 
			std::uniform_real_distribution dist(rangeMin, rangeMax);
			return static_cast<float>(dist(_Gen()));
		}

		inline static int Range(int rangeMin, int rangeMax) {
			std::uniform_int_distribution dist(rangeMin, rangeMax);
			return dist(_Gen());
		}
	private:
		inline static std::mt19937& _Gen() { 
			static std::random_device d;
			static std::mt19937 generator(d());
			return generator;
		}
	};
} //namespace
