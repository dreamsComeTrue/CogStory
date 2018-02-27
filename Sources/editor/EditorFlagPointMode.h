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

        void DrawFlagPoints (float mouseX, float mouseY);
        bool MoveSelectedFlagPoint ();
        std::string GetFlagPointUnderCursor (int mouseX, int mouseY);
        void InsertFlagPointAtCursor (int mouseX, int mouseY);
        bool RemoveFlagPointUnderCursor (int mouseX, int mouseY);

        std::string GetFlagPoint () { return m_FlagPoint; }
        void SetFlagPoint (const std::string& pointName) { m_FlagPoint = pointName; }

        std::string GetFlagPointName () { return m_FlagPointName; }
        void SetFlagPointName (const std::string& name) { m_FlagPointName = name; }

        void SetAskFlagPoint (bool ask) { m_AskFlagPoint = ask; }

    private:
        Editor* m_Editor;

        bool m_AskFlagPoint;
        std::string m_FlagPointName;
        std::string m_FlagPoint;

        bool m_DrawConnection;
    };
}

#endif //   __EDITOR_FLAG_POINT_MODE_H__
