// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_PHYS_MODE_H__
#define __EDITOR_PHYS_MODE_H__

#include "Common.h"

namespace aga
{
    class Editor;
    class EditorTileMode;
    class MainLoop;

    class EditorPhysMode
    {
        friend class Editor;
        friend class EditorTileMode;

    public:
        EditorPhysMode (Editor* editor);
        virtual ~EditorPhysMode ();

    private:
        void DrawPhysBody (float mouseX, float mouseY);

        bool MoveSelectedPhysPoint ();

        void InsertPhysPointAtCursor (int mouseX, int mouseY);
        Point* GetPhysPointUnderCursor (int mouseX, int mouseY);
        bool RemovePhysPointUnderCursor (int mouseX, int mouseY);

        void ResetSettings ();

    private:
        Editor* m_Editor;

        Point* m_PhysPoint;
        int m_PhysPointIndex;
        std::vector<Point>* m_PhysPoly;
    };
}

#endif //   __EDITOR_PHYS_MODE_H__
