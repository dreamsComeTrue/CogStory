// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_SCRIPT_WINDOW_H__
#define __EDITOR_SCRIPT_WINDOW_H__

#include "Common.h"
#include "Entity.h"

#include <Gwork/Controls.h>

namespace aga
{
    class Editor;

    class EditorScriptWindow : public Gwk::Event::Handler
    {
    public:
        EditorScriptWindow (Editor* editor, Gwk::Controls::Canvas* canvas);
        virtual ~EditorScriptWindow ();

        void Show (std::function<bool(void)> OnAcceptFunc, std::function<bool(void)> OnCancelFunc);
        bool GetResult ();

        std::string GetName () const { return m_Name; }
        std::string GetPath () const { return m_Path; }

    private:
        void OnBrowse ();
        void OnAccept ();
        void OnCancel ();

        void OnNameEdit (Gwk::Controls::Base* control);
        void OnPathEdit (Gwk::Controls::Base* control);

    private:
        Editor* m_Editor;
        Gwk::Controls::WindowControl* m_SceneWindow;
        Gwk::Controls::TextBox* m_PathTextBox;

        std::string m_Name;
        std::string m_Path;

        std::string m_FileName;
        bool m_Result;

        std::function<bool(void)> m_OnAcceptFunc;
        std::function<bool(void)> m_OnCancelFunc;
    };
}

#endif //   __EDITOR_SCRIPT_WINDOW_H__
