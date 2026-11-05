#pragma once
#if defined(COMPILE_WITH_EDITOR)
#include "../Widget.h"
#include "Core/SubSystems/ImGui/EditorSession.h"

namespace SE
{
	// @brief The main widget that all other widgets are docked into
	class MainWidget : public Widget
	{
	public:
		explicit MainWidget(EditorSession& session)
		    : Widget("MainWidget"), m_session(session){};

		void render() override;
		void shutdown() override;

		// @brief Prepares the main widget for rendering
		void beginEditorFrame();
		// @brief Finishes the main widget rendering and performs any queued actions
		void finishEditorFrame();

		static bool isSceneEditingBlocked() { return s_editsBlocked; }

	private:
		// @brief The actions that can be performed in the main widget
		enum class Action { None, New, Open, Save, SaveAs, Reload };
		// @brief The dialogs that can be shown in the main widget
		enum class Dialog { None, Confirm, Open, SavePath };

		// @brief Queues an action to be performed
		// @param action The action to be performed
		void queueAction(Action action);
		// @brief Performs queued action
		void performAction();
		// @brief Finishes the queued action
		void finishAction();

		// @brief Prepares a dialog for the specified action
		// @param dialog The dialog to be shown
		void prepDialog(Dialog dialog);
		// @brief Renders any open dialogs, call this after rendering the main menu bar
		void drawActiveDialog();
		
		// @brief Refreshes the list of scene files in the current folder
		void refreshSceneFiles();

		// @brief Saves the current scene to the specified path
		// @param path The path to save the scene to
		// @return True if the scene was saved successfully, false otherwise
		bool saveScene(const std::string& path);

		Action m_action = Action::None;
		int m_actionFrame = -1;

		Dialog m_dialog = Dialog::None;
		bool m_openDialog = false;

		bool m_confirmOverwrite = false;

		uint64_t m_actionRevision = 0;

		std::string m_folder = "Assets/Data";
		std::string m_filename;
		std::string m_selectedPath;
		std::string m_error;
		std::vector<std::string> m_sceneFiles;

		static inline bool s_editsBlocked = false;

		EditorSession& m_session;
	};
} // namespace SE
#endif
