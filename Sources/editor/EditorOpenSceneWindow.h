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

        void Show ();
        void OnOpen (Gwk::Controls::Base*);
        void OnCancel ();
        void OnEdit (Gwk::Controls::Base* control);

    private:
        Editor* m_Editor;
        Gwk::Controls::WindowControl* m_SceneWindow;
        std::string m_FileName;
    };
}

#endif //   __EDITOR_OPEN_SCENE_WINDOW_H__
