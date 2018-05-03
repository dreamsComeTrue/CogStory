// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_COMPONENT_WINDOW_H__
#define __EDITOR_COMPONENT_WINDOW_H__

#include "Common.h"
#include "Entity.h"

#include <Gwork/Controls.h>

namespace aga
{
    class Editor;

    class EditorComponentWindow : public Gwk::Event::Handler
    {
    public:
        EditorComponentWindow (Editor* editor, Gwk::Controls::Canvas* canvas);
        virtual ~EditorComponentWindow ();

        void Show (std::function<bool(void)> OnAcceptFunc, std::function<bool(void)> OnCancelFunc);
        bool GetResult ();

        std::string GetName () const { return m_Name; }
        std::string GetTypeName () const { return m_Type; }

    private:
        void OnAccept ();
        void OnCancel ();

        void OnNameEdit (Gwk::Controls::Base* control);
        void OnTypeEdit (Gwk::Controls::Base* control);

    private:
        Editor* m_Editor;
        Gwk::Controls::WindowControl* m_SceneWindow;
        Gwk::Controls::TextBox* m_PathTextBox;

        std::string m_Name;
        std::string m_Type;

        bool m_Result;

        std::function<bool(void)> m_OnAcceptFunc;
        std::function<bool(void)> m_OnCancelFunc;
    };
}

#endif //   __EDITOR_COMPONENT_WINDOW_H__

