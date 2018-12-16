// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_SAVESCENE_WINDOW_H__
#define __EDITOR_SAVESCENE_WINDOW_H__

#include "Common.h"

namespace aga
{
    class Editor;

    class EditorSaveSceneWindow
    {
    public:
        EditorSaveSceneWindow (Editor* editor);
        virtual ~EditorSaveSceneWindow ();

        void Show (const std::string& filePath, std::function<void(std::string)> OnAcceptFunc,
            std::function<void(std::string)> OnCancelFunc);

        std::string GetSceneName () const;

        void Render ();

        bool IsVisible ();

    private:
        void OnAccept ();
        void OnCancel ();

    private:
        Editor* m_Editor;
        bool m_IsVisible;
        bool m_BrowseButtonPressed;
        char m_SceneName[100];

        std::function<void(std::string)> m_OnAcceptFunc;
        std::function<void(std::string)> m_OnCancelFunc;
    };
}

#endif //   __EDITOR_SAVESCENE_WINDOW_H__
