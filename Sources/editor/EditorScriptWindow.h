// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_SCRIPT_WINDOW_H__
#define __EDITOR_SCRIPT_WINDOW_H__

#include "Common.h"

#include "TextEditor.h"

namespace aga
{
	class Editor;

	struct FileEntry
	{
		std::string Name;
		std::string Path;
		std::string Text;
		bool Modified = false;

		TextEditor TextEditorControl;
	};

	class EditorScriptWindow
	{
	public:
		EditorScriptWindow (Editor* editor);
		virtual ~EditorScriptWindow ();

		void Show ();

		void ProcessEvent (ALLEGRO_EVENT* event);
		void RenderUI ();

		bool IsVisible () const;

		void AddEntry (const std::string& dataPath);

		std::vector<FileEntry>& GetEntries ();

	private:
		void OpenSelectedFile ();
		void SaveCurrentFile ();

	private:
		Editor* m_Editor;
		bool m_IsVisible;

		bool m_OpenButtonPressed;

		std::vector<FileEntry> m_Entries;
		int m_SelectedEntry;
	};
}

#endif //   __EDITOR_SCRIPT_WINDOW_H__
