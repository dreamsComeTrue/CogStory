// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_OPENSCENE_WINDOW_H__
#define __EDITOR_OPENSCENE_WINDOW_H__

#include "Common.h"

namespace aga
{
    class Editor;

    class EditorOpenSceneWindow
    {
    public:
        EditorOpenSceneWindow (Editor* editor);
        virtual ~EditorOpenSceneWindow ();

        void Show (const std::string& filePath, std::function<void(std::string)> OnAcceptFunc,
            std::function<void(std::string)> OnCancelFunc);

        std::string GetSceneName () const;

        void ProcessEvent (ALLEGRO_EVENT* event);
        void Render ();

        bool IsVisible ();

        std::vector<std::string>& GetRecentFileNames ();
        void AddRecentFileName (const std::string& name);

    private:
        void OnAccept ();
        void OnCancel ();

    private:
        Editor* m_Editor;
        bool m_IsVisible;
        char m_SceneName[100];
        int m_ItemCurrent;

        bool m_ScheduleClosed;

        std::vector<std::string> m_RecentFileNames;

        std::function<void(std::string)> m_OnAcceptFunc;
        std::function<void(std::string)> m_OnCancelFunc;
    };
}

#endif //   __EDITOR_OPENSCENE_WINDOW_H__
