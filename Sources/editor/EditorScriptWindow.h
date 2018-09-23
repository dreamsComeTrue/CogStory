// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_SCRIPT_WINDOW_H__
#define __EDITOR_SCRIPT_WINDOW_H__

#include "Common.h"
#include "Entity.h"

#include "imgui.h"

namespace aga
{
    class Editor;

    class EditorScriptWindow
    {
    public:
        EditorScriptWindow (Editor* editor);
        virtual ~EditorScriptWindow ();

        void Render ();

        std::string GetName () const { return m_Name; }
        std::string GetPath () const { return m_Path; }

    private:
        void OnBrowse ();

    private:
        Editor* m_Editor;

        char m_Name[100];
        char m_Path[100];
    };
}

#endif //   __EDITOR_SCRIPT_WINDOW_H__
