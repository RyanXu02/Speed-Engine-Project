#pragma once

#if defined(COMPILE_WITH_EDITOR)

namespace SE
{
	// @brief Represents the state of the editor session
	class EditorSession
	{
	public:
		// @brief Returns the set of currently selected entity IDs
		const std::unordered_set<uint32_t>& selectedEntities() const { return m_selectedEntities; }

		// @brief Returns the ID of the currently active entity, or 0 if no entity is active
		uint32_t activeEntity() const { return m_activeEntity; }

		void requestSavePrefab(uint32_t id) { m_savePrefabRequest = id; }
		uint32_t takeSavePrefabRequest()
		{
			const uint32_t id = m_savePrefabRequest;
			m_savePrefabRequest = 0;
			return id;
		}

		// @brief Selects the specified entity ID, clearing any previous selection
		void select(uint32_t id)
		{
			clearSelection();
			if (id)
			{
				m_selectedEntities.insert(id);
				m_activeEntity = id;
			}
		}

		// @brief Toggles the selection state of the specified entity ID
		void toggle(uint32_t id)
		{
			if (!id)
				return;
			if (m_selectedEntities.contains(id))
				removeSelected(id);
			else
			{
				m_selectedEntities.insert(id);
				m_activeEntity = id;
			}
		}

		// @brief Clears the current selection of entities
		void clearSelection()
		{
			m_selectedEntities.clear();
			m_activeEntity = 0;
		}

		// @brief Removes the specified entity ID from the current selection
		void removeSelected(uint32_t id)
		{
			m_selectedEntities.erase(id);
			if (m_activeEntity == id)
				m_activeEntity = m_selectedEntities.empty() ? 0 : *m_selectedEntities.begin();
		}

		bool isDirty() const { return m_dirty; }
		void markDirty() { m_dirty = true; }
		void clearDirty() { m_dirty = false; }

		// @brief Returns the current scene revision, which is incremented whenever the scene is modified
		uint64_t sceneRevision() const { return m_sceneRevision; }

		// @brief Synchronizes the session with the specified scene revision, clearing selection and dirty state if the revision has changed
		void syncSceneRevision(uint64_t revision)
		{
			if (m_sceneRevision == revision)
				return;
			m_sceneRevision = revision;
			m_savePrefabRequest = 0;
			clearSelection();
			clearDirty();
		}

	private:
		std::unordered_set<uint32_t> m_selectedEntities;

		uint32_t m_activeEntity = 0;
		uint32_t m_savePrefabRequest = 0;

		bool m_dirty = false;

		uint64_t m_sceneRevision = 0;
	};
}
#endif
