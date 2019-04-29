// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_SCRIPT_SELECT_WINDOW_H__
#define __EDITOR_SCRIPT_SELECT_WINDOW_H__

#include "Common.h"

namespace aga
{
    class Editor;

    class EditorScriptSelectWindow
    {
    public:
        EditorScriptSelectWindow (Editor* editor);
        virtual ~EditorScriptSelectWindow ();

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
        bool m_BrowseButtonPressed;

        char m_Name[100];
        char m_Path[100];

        std::function<void(std::string, std::string)> m_OnAcceptFunc;
        std::function<void(std::string, std::string)> m_OnCancelFunc;
    };
}

#endif //   __EDITOR_SCRIPT_SELECT_WINDOW_H__
