// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_OPEN_SCENE_WINDOW_H__
#define __EDITOR_OPEN_SCENE_WINDOW_H__

#include "Common.h"

#include <Gwork/Controls.h>

namespace aga
{
    class Editor;

    class EditorOpenSceneWindow : public Gwk::Event::Handler
    {
    public:
        EditorOpenSceneWindow (Editor* editor, Gwk::Controls::Canvas* canvas, const std::string& fileName);

        void Show (const std::string& filePath = "");

        void SetFileName (const std::string& fileName) { m_FileName = fileName; }
        std::string GetFileName () const { return m_FileName; }

        Gwk::Controls::WindowControl* GetSceneWindow () { return m_SceneWindow; }
        std::vector<std::string> GetRecentFileNames ();
        void AddRecentFileName (const std::string& name);

    private:
        void OnOpen ();
        void OnCancel ();
        void OnEdit (Gwk::Controls::Base* control);
        void OnBrowse ();
        void OnRecentFile ();

    private:
        Editor* m_Editor;
        Gwk::Controls::WindowControl* m_SceneWindow;
        Gwk::Controls::TextBox* m_PathTextBox;
        Gwk::Controls::ListBox* m_RecentFiles;
        std::string m_FileName;
    };
}

#endif //   __EDITOR_OPEN_SCENE_WINDOW_H__
