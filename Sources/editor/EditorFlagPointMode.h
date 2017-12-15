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
        friend class Editor;
        friend class EditorFlagPointWindow;
        friend class EditorTileMode;

    public:
        EditorFlagPointMode (Editor* editor);
        virtual ~EditorFlagPointMode ();

    private:
        void DrawFlagPoints (float mouseX, float mouseY);
        bool MoveSelectedFlagPoint ();
        std::string GetFlagPointUnderCursor (int mouseX, int mouseY);
        void InsertFlagPointAtCursor (int mouseX, int mouseY);
        bool RemoveFlagPointUnderCursor (int mouseX, int mouseY);

    private:
        Editor* m_Editor;

        bool m_AskFlagPoint;
        std::string m_FlagPointName;
        std::string m_FlagPoint;

        bool m_DrawConnection;
    };
}

#endif //   __EDITOR_FLAG_POINT_MODE_H__
