// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_COMPONENT_WINDOW_H__
#define __EDITOR_COMPONENT_WINDOW_H__

#include "Common.h"
#include "Entity.h"

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
        bool GetResult () { return m_Result; }

        std::string GetName () const { return m_Name; }
        std::string GetTypeName () const { return m_Types[m_SelectedType]; }

        void Render ();

        bool IsVisible () { return m_IsVisible; }

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

        bool m_Result;

        std::function<void(std::string, std::string)> m_OnAcceptFunc;
        std::function<void(std::string, std::string)> m_OnCancelFunc;
    };
}

#endif //   __EDITOR_COMPONENT_WINDOW_H__
