#pragma once

#if defined(COMPILE_WITH_EDITOR)
#include "Core/Scene/Entity/Entity.h"

namespace SE
{
	template<typename T>
	concept JsonConvertible = requires(const T& source, T& destination, const nlohmann::json& json)
	{
		{ nlohmann::json(source) } -> std::same_as<nlohmann::json>;
		{ json.get_to(destination) } -> std::same_as<T&>;
	};

	class ComponentInspectorRegistry
	{
	public:
		// @brief A component type to be displayed in the properties widget
		struct Entry
		{
			std::type_index type;
			std::string name;
			bool addable;
			bool removable;
			std::function<bool(Entity&)> has;
			std::function<void(Entity&)> add;
			std::function<void(Entity&)> remove;
			std::function<void(Entity&)> render;
		};

		// @brief Registers a component type to be displayed in the properties widget
		// @param T The type of the component to be displayed in the properties widget, must be JsonConvertible
		// @param name The name of the component to be displayed in the properties widget
		// @param removable Whether the component can be removed from the entity
		// @param editor The function that renders the component's properties in the properties widget. If editor is not provided, the registry will attempt to allow it for editing.
		// @param onEdited The function that is called when the component's properties are edited. This is useful for components that do not have a custom editor function provided.
		template<typename T> requires JsonConvertible<T>
		void registerComponent(
			std::string name, 
			bool removable = true,
			std::function<void(Entity&, T&)> editor = {},
			std::function<void(Entity&, T&)> onEdited = {})
		{
			_addEntry<T>(std::move(name), std::is_default_constructible_v<T>, removable,
				[editor, onEdited](Entity& entity)
				{
					auto* component = entity.getComponent<T>();
					if (!component) return;
					if (editor)
					{
						editor(entity, *component);
						return;
					}
					nlohmann::json properties = *component;
					if (_editComponentJson("Properties", properties))
					{
						properties.get_to(*component);
						if (onEdited) onEdited(entity, *component);
					}
				});
		}

		// @brief Registers a read-only component type to be displayed in the properties widget
		// @param name The name of the component to be displayed in the properties widget
		// @param editor The function that renders the component's properties in the properties widget
		template<typename T>
		void registerReadOnly(std::string name, std::function<void(Entity&, T&)> editor)
		{
			_addEntry<T>(std::move(name), false, false,
				[editor](Entity& entity)
				{
				    auto* component = entity.getComponent<T>();
				    if (!component) return;
				    if (editor)
				    {
					    editor(entity, *component);
					    return;
				    }
				});
		}

		// @brief Returns the component types regiestered in the properties widget
		const std::vector<Entry>& getEntries() const { return m_entries; }

	private:
		// @brief Used to render a component's properties in the properties widget
		// @brief This function is used when a component does not have a custom editor function provided
		// @param label The label to be displayed
		// @param value The component's properties to be displayed
		static bool _editComponentJson(const char* label, nlohmann::json& value);
		
		template<typename T>
		void _addEntry(std::string name, bool addable, bool removable, std::function<void(Entity&)> render)
		{
			// component already registered
			if (std::ranges::find(m_entries, std::type_index(typeid(T)), &Entry::type) != m_entries.end())
				return;

			m_entries.push_back(
				{
					.type = typeid(T),
					.name = std::move(name),
					.addable = addable,
					.removable = removable,
					.has = [](Entity& entity)
					{
						return entity.getComponent<T>() != nullptr;
					},
					.add = [](Entity& entity)
					{
						if constexpr (std::is_default_constructible_v<T>)
							if (!entity.getComponent<T>())
								entity.addComponent<T>();
					},
					.remove = [](Entity& entity)
					{
						entity.removeComponent<T>(); 
					},
					.render = std::move(render)
				}
			);
		}

		std::vector<Entry> m_entries;
	};
}
#endif
