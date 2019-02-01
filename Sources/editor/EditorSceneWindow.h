// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_SCENE_WINDOW_H__
#define __EDITOR_SCENE_WINDOW_H__

#include "Common.h"

#include "imgui.h"

namespace aga
{
    class Editor;

    class EditorSceneWindow
    {
    public:
        EditorSceneWindow (Editor* editor);
        virtual ~EditorSceneWindow ();

        void Show ();
        void RenderUI ();

        bool IsVisible () const;

    private:
        void OnReloadScript (const std::string& scriptName);

        void OnNameEdit ();

    private:
        Editor* m_Editor;
        bool m_IsVisible;

        char m_SceneName[100];
        ImVec4 m_BackColor;
    };
}

#endif //   __EDITOR_SCENE_WINDOW_H__
