#pragma once

#include "Logger/LoggerProxy.h"

#include <nlohmann/json.hpp>

// @brief Describe which fields are serialized and how an individual component converts to and from JSON
// @param ClassName The name of the class to define serialization for
// @param ... The fields of the class to serialize
#define DEFINE_TYPE(ClassName, ...)										\
	static const char* getStaticType()									\
	{																	\
		return #ClassName;												\
	}																	\
	template <typename Member>											\
	static void visitFields(Member&& member)							\
	{																	\
		__VA_ARGS__;													\
	}																	\
	friend void to_json(nlohmann::json& j, const ClassName& obj)		\
	{																	\
		j = nlohmann::json::object();									\
		auto resolver = [&j, &obj](const char* name, auto memberPtr) {	\
			j[name] = obj.*memberPtr;									\
		};																\
		ClassName::visitFields(resolver);								\
	}																	\
	friend void from_json(const nlohmann::json& j, ClassName& obj)		\
	{																	\
		auto resolver = [&j, &obj](const char* name, auto memberPtr) {	\
			if (j.contains(name)){										\
				j.at(name).get_to(obj.*memberPtr);						\
			}															\
		};																\
		ClassName::visitFields(resolver);								\
	}

// @brief Register the factory used to create the type from JSON
// @brief Component life cycle stuff are registered in the EntityRegistry, this is just for serialization and deserialization
// @param Type the type to register
// @param ComponentFactory the factory function that takes an entity ID and a JSON object and creates the component
#define REGISTER_TYPE(Type, ComponentFactory)				\
	m_typeResolver.Register<Type>(#Type, ComponentFactory);

namespace SE
{
	template<typename T>
	struct TypeSerializer
	{
		static nlohmann::json Serialize(const std::any& instance)
		{
			if (!instance.has_value())
			{
				return nlohmann::json::object();
			}
			//return nlohmann::json(*static_cast<const T*>(instance));
			return nlohmann::json(std::any_cast<const T&>(instance));
		}
		static void Deserialize(void* instance, const nlohmann::json& j)
		{
			if (!instance)
			{
				return;
			}
			//j.get_to(*static_cast<const T*>(instance));
			nlohmann::adl_serializer<T>::from_json(j, *static_cast<T*>(instance));
		}
	};

	typedef std::pair<std::any, std::any> SerializerPair; //first serializer, second: deserializer
	typedef std::function<void(uint32_t, const nlohmann::json&)> ComponentFactory;

	class MetaType
	{
	public:
		MetaType(std::any data, ComponentFactory constructor)
		    : m_serializer(std::move(data)),
		      m_constructor(std::move(constructor))
		{}

		bool isValid() { return m_serializer.has_value(); };

		// manual compile time, object creation from the json, aka: static deserializer, more type safe
		// but the serializer uses registered invoke function instead as it needs to inspect type at run time
		template<typename T>
		T resolve(nlohmann::json j)
		{
			auto* serializer = std::any_cast<SerializerPair>(&m_serializer);
			if (!serializer)
			{
				throw std::runtime_error("MetaType cast: Failed to cast to desired type");
			}

			//using DeserializeFunc = void (*)(void*, const nlohmann::json&);
			typedef void (*DeserializeFunc)(void*, const nlohmann::json&);
			auto deserializeFunc = std::any_cast<DeserializeFunc>(serializer->second);
			assert(deserializeFunc && "MetaType cast: failed to cast deserializer function");

			T instance;
			deserializeFunc(&instance, j);

			return instance;
		}

		nlohmann::json serialize(const std::any& instance)
		{
			auto* serializer = std::any_cast<SerializerPair>(&m_serializer);
			if (!serializer)
			{
				throw std::runtime_error("MetaType cast: Failed to cast to desired type");
			}

			typedef nlohmann::json (*SerializeFunc)(const std::any&);
			auto serializeFunc = std::any_cast<SerializeFunc>(serializer->first);
			assert(serializeFunc && "MetaType cast: failed to cast serialize function");

			return serializeFunc(instance);
		}

		// invoke the registered construction function
		void invoke(uint32_t entityID, const nlohmann::json& data)
		{
			if (m_constructor)
			{
				m_constructor(entityID, data);
			}
		}


	private:
		std::any m_serializer;
		ComponentFactory m_constructor;
	};


	class MetaTypeResolver
	{
	public:
		MetaType Get(std::string_view prop) const
		{
			auto typeSerializerIt = m_typeSerializers.find(prop.data());
			auto typeConstructorIt = m_typeFactory.find(prop.data());
			if (typeSerializerIt != m_typeSerializers.end())
			{
				if (typeConstructorIt != m_typeFactory.end())
				{
					return MetaType(typeSerializerIt->second, typeConstructorIt->second);
				}
				else
				{
					return MetaType(typeSerializerIt->second, nullptr);
				}
			}
			return MetaType{ std::any{}, {} };
		}

		template<typename T>
		void Register(std::string_view name, ComponentFactory loadFunc)
		{
			m_typeSerializers.try_emplace(name.data(), &TypeSerializer<T>::Serialize, &TypeSerializer<T>::Deserialize);
			m_typeFactory.try_emplace(name.data(), std::move(loadFunc));
		}

	private:
		std::unordered_map<std::string, SerializerPair> m_typeSerializers;
		std::unordered_map<std::string, ComponentFactory> m_typeFactory;
	};



	class Serializable
	{
	public:
		virtual ~Serializable() = default;
		virtual void serialize(nlohmann::json& j) const = 0;
		virtual void deserialize(const nlohmann::json& j) = 0;

		static bool loadJson(const std::string& path, const LoggerProxy& logger, nlohmann::json& json);
		static std::string demagleTypeInfo(const std::type_index& typeInfo);

	protected:
		Serializable();

		MetaTypeResolver m_typeResolver;

	private:
	};
}
