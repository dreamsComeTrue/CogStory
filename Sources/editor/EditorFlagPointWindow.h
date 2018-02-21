// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_FLAG_POINT_WINDOW_H__
#define __EDITOR_FLAG_POINT_WINDOW_H__

#include "Common.h"

#include <Gwork/Controls.h>

namespace aga
{
    class Editor;

    class EditorFlagPointWindow : public Gwk::Event::Handler
    {
        friend class Editor;

    public:
        EditorFlagPointWindow (Editor* editor, Gwk::Controls::Canvas* canvas);

        void Show ();
        void OnAccept (Gwk::Controls::Base*);
        void OnCancel ();
        void OnEdit (Gwk::Controls::Base* control);

    private:
        Editor* m_Editor;
        Gwk::Controls::WindowControl* m_SceneWindow;
    };
}

#endif //   __EDITOR_FLAG_POINT_WINDOW_H__
