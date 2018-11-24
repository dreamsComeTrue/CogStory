// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_FLAG_POINT_MODE_H__
#define __EDITOR_FLAG_POINT_MODE_H__

#include "Common.h"

namespace aga
{
    class Editor;
    struct FlagPointWindow;

    class EditorFlagPointMode
    {
    public:
        EditorFlagPointMode (Editor* editor);
        virtual ~EditorFlagPointMode ();

        void Render ();
        bool MoveSelectedFlagPoint ();
        std::string GetFlagPointUnderCursor (int mouseX, int mouseY);
        void InsertFlagPointAtCursor (int mouseX, int mouseY);
        bool RemoveFlagPointUnderCursor (int mouseX, int mouseY);

        std::string GetFlagPoint ();
        void SetFlagPoint (const std::string& name);

        void SetAskFlagPoint (bool ask);

        void RenderUI ();

    private:
        Editor* m_Editor;

        bool m_AskFlagPoint;
        std::string m_FlagPoint;
        char m_FlagPointWindow[100];

        bool m_Editing;
        bool m_DrawConnection;
    };
}

#endif //   __EDITOR_FLAG_POINT_MODE_H__
