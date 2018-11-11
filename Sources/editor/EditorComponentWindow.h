// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_COMPONENT_WINDOW_H__
#define __EDITOR_COMPONENT_WINDOW_H__

#include "Common.h"

namespace aga
{
    class Editor;

    class EditorComponentWindow
    {
    public:
        EditorComponentWindow (Editor* editor);
        virtual ~EditorComponentWindow ();

        void Show (std::function<void(std::string, std::string)> OnAcceptFunc,
            std::function<void(std::string, std::string)> OnCancelFunc);

        std::string GetName () const;
        std::string GetTypeName () const;

        void Render ();

        bool IsVisible ();

    private:
        void OnAccept ();
        void OnCancel ();

    private:
        Editor* m_Editor;

        std::vector<std::string> m_Types;
        int m_SelectedType;

        bool m_IsVisible;

        char m_Name[100];
        char m_Type[100];

        std::function<void(std::string, std::string)> m_OnAcceptFunc;
        std::function<void(std::string, std::string)> m_OnCancelFunc;
    };
}

#endif //   __EDITOR_COMPONENT_WINDOW_H__
