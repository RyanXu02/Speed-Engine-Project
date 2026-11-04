#include "pch.h"

#include "Serializable.h"

namespace SE
{
	bool Serializable::loadJson(const std::string& path, const LoggerProxy& logger, nlohmann::json& json)
	{
		std::ifstream file(path);
		if (!file.is_open())
		{
			logger.warn("Fail to open json: {}", path);
			return false;
		}

		// noexcept, returns a discarded json object on error
		json = nlohmann::json::parse(file, nullptr, false);

		if (json.is_discarded())
		{
			logger.warn("Fail to parse json: {}", path);
			return false;
		}

		return true;
	}

	// type info conversion to string is not consistent across machine nor compiler
	// this function makes a big assumption of using MSVC and Window x64
	std::string Serializable::demagleTypeInfo(const std::type_index& type)
	{
		std::string name = type.name();

		// strip prefix if present
		if (name.rfind("class ", 0) == 0)
		{
			name.erase(0, 6);
		}
		else if (name.rfind("struct ", 0) == 0)
		{
			name.erase(0, 7);
		}

		// strip namespace
		size_t lastColon = name.rfind("::");
		if (lastColon != std::string::npos)
		{
			name.erase(0, lastColon + 2);
		}

		return name;
	}

	Serializable::Serializable() {}
}
