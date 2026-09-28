#pragma once

#include <nlohmann/json.hpp>
#include <unordered_map>
#include <typeindex>
#include <functional>
#include <string>
#include "Serializable.h"

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
