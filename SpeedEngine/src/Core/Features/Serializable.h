#pragma once

#include "Logger/LoggerProxy.h"

#include <nlohmann/json.hpp>

//move away from nlohman intrusive type, use a custom one
//while maintaining nlohman type registration
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

// register the behavior of the type on construction and destruction
#define REGISTER_TYPE(Type, ConstructorFunc, DesctructorFunc)				\
	m_typeResolver.Register<Type>(#Type, ConstructorFunc, DesctructorFunc);	\

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
			return nlohmann::json(std::any_cast<T>(instance));
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
	typedef std::function<void(uint32_t, const nlohmann::json&)> ConstructorFunc;
	typedef std::function<void(uint32_t, const nlohmann::json&)> DestructorFunc;
	typedef std::pair<ConstructorFunc, DestructorFunc> TypeFactory;

	class MetaType
	{
	public:
		MetaType(std::any data, std::any constructor)
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
		template<typename InvokeFunc, typename... Args>
		void invoke(Args&&... args)
		{
			if (!m_constructor.has_value())
			{
				return;
			}

			auto* typeConstructor = std::any_cast<TypeFactory>(&m_constructor);
			if (!typeConstructor)
			{
				throw std::runtime_error("MetaType invoke: Failed to cast to desired type");
			}

			if (typeid(InvokeFunc) == typeid(ConstructorFunc))
			{
				auto constructorFunc = typeConstructor->first;
				constructorFunc(std::forward<Args>(args)...);
			}
			else if (typeid(InvokeFunc) == typeid(DestructorFunc))
			{
				auto destructorFunc = typeConstructor->second;
				destructorFunc(std::forward<Args>(args)...);
			}
			else
			{
				throw std::runtime_error("MetaType invoke: Failed to invoke to desired function");
			}
		}


	private:
		std::any m_serializer;
		std::any m_constructor;
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
			return MetaType{ std::any{}, std::any{} };
		}

		template<typename T>
		void Register(std::string_view name, ConstructorFunc loadFunc, DestructorFunc unloadFunc)
		{
			m_typeSerializers.try_emplace(name.data(), &TypeSerializer<T>::Serialize, &TypeSerializer<T>::Deserialize);
			m_typeFactory.try_emplace(name.data(), loadFunc, unloadFunc);
		}

	private:
		std::unordered_map<std::string, SerializerPair> m_typeSerializers;
		std::unordered_map<std::string, TypeFactory> m_typeFactory;
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
