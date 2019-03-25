// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_OPENSCENE_WINDOW_H__
#define __EDITOR_OPENSCENE_WINDOW_H__

#include "Common.h"

namespace aga
{
	class Editor;

	struct RecentFilesGroup
	{
		std::string Name;
		std::vector<std::string> Paths;
		int CurrentItem;
	};

	class EditorOpenSceneWindow
	{
	public:
		EditorOpenSceneWindow (Editor* editor);
		virtual ~EditorOpenSceneWindow ();

		void Show (const std::string& filePath, std::function<void(std::string)> OnAcceptFunc,
			std::function<void(std::string)> OnCancelFunc);

		std::string GetSceneName () const;

		void ProcessEvent (ALLEGRO_EVENT* event);
		void Render ();

		bool IsVisible ();

		std::vector<std::string>& GetRecentFileNames ();
		void AddRecentFileName (const std::string& name);

	private:
		void OnAccept ();
		void OnCancel ();
		void ComputeGroups ();

	private:
		Editor* m_Editor;
		bool m_IsVisible;
		char m_SceneName[100];
		int m_ItemCurrent;
		int m_SelectedGroup;

		bool m_ScheduleClosed;
		bool m_BrowseButtonPressed;

		std::vector<std::string> m_RecentFileNames;
        std::vector<RecentFilesGroup> m_Groups;

		std::function<void(std::string)> m_OnAcceptFunc;
		std::function<void(std::string)> m_OnCancelFunc;
	};
}

#endif //   __EDITOR_OPENSCENE_WINDOW_H__
