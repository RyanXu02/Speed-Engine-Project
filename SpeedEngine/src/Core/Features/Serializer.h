#pragma once

#include "Serializable.h"

#include <functional>
#include <nlohmann/json.hpp>
#include <string>
#include <typeindex>
#include <unordered_map>

namespace SE
{
	class Serializer
	{
	public:
		~Serializer();

		//static Serializer& Instance();

		//void addSerializer(std::type_index type);

	private:
		Serializer();

		std::unordered_map<std::type_index, std::unique_ptr<Serializable>> m_serializers;
	};

}
