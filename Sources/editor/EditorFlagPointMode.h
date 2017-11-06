// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_FLAG_POINT_MODE_H__
#define __EDITOR_FLAG_POINT_MODE_H__

#include "Common.h"

namespace aga
{
    class Editor;

    class EditorFlagPointMode
    {
        friend class Editor;
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
        char m_FlagPointName[64];
        std::string m_FlagPoint;
    };
}

#endif //   __EDITOR_FLAG_POINT_MODE_H__