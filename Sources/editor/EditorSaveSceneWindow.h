// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_SAVE_SCENE_WINDOW_H__
#define __EDITOR_SAVE_SCENE_WINDOW_H__

#include "Common.h"

#include <Gwork/Controls.h>

namespace aga
{
    class Editor;

    class EditorSaveSceneWindow : public Gwk::Event::Handler
    {
    public:
        EditorSaveSceneWindow (Editor* editor, Gwk::Controls::Canvas* canvas, const std::string& fileName);

        void Show (const std::string& path = "");
        void OnBrowse ();
        void OnSave (Gwk::Controls::Base*);
        void OnCancel ();
        void OnEdit (Gwk::Controls::Base* control);

        std::string GetFileName () const { return m_FileName; }

        Gwk::Controls::WindowControl* GetSceneWindow () { return m_SceneWindow; }

    private:
        Editor* m_Editor;
        Gwk::Controls::WindowControl* m_SceneWindow;
        Gwk::Controls::TextBox* m_PathTextBox;
        std::string m_FileName;
    };
}

#endif //   __EDITOR_SAVE_SCENE_WINDOW_H__
