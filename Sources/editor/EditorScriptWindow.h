// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_SCRIPT_WINDOW_H__
#define __EDITOR_SCRIPT_WINDOW_H__

#include "Common.h"

namespace aga
{
    class Editor;

    class EditorScriptWindow
    {
    public:
        EditorScriptWindow (Editor* editor);
        virtual ~EditorScriptWindow ();

        void Show (std::function<void(std::string, std::string)> OnAcceptFunc,
            std::function<void(std::string, std::string)> OnCancelFunc);

        void Render ();

        std::string GetName () const;
        std::string GetPath () const;

        bool IsVisible ();

    private:
        void OnAccept ();
        void OnCancel ();

        void OnBrowse ();

    private:
        Editor* m_Editor;
        bool m_IsVisible;

        char m_Name[100];
        char m_Path[100];

        std::function<void(std::string, std::string)> m_OnAcceptFunc;
        std::function<void(std::string, std::string)> m_OnCancelFunc;
    };
}

#endif //   __EDITOR_SCRIPT_WINDOW_H__
