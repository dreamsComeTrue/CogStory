// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_PHYS_MODE_H__
#define __EDITOR_PHYS_MODE_H__

#include "Common.h"

namespace aga
{
    class Editor;
    class EditorActorMode;
    class MainLoop;

    class EditorPhysMode
    {
    public:
        EditorPhysMode (Editor* editor);
        virtual ~EditorPhysMode ();

        void DrawPhysPoints (int mouseX, int mouseY);
        void DrawGuideLines ();

        bool MoveSelectedPhysPoint ();

        void InsertPhysPointAtCursor (int mouseX, int mouseY);
        Point* GetPhysPointUnderCursor (int mouseX, int mouseY);
        bool RemovePhysPointUnderCursor (int mouseX, int mouseY);

        void ResetSettings ();

        Point* GetPhysPoint ();
        void SetPhysPoint (Point* point);

        std::vector<Point>* GetPhysPoly ();
        void SetPhysPoly (std::vector<Point>* poly);

    private:
        Editor* m_Editor;

        Point* m_PhysPoint;
        int m_PhysPointIndex;
        std::vector<Point>* m_PhysPoly;
    };
}

#endif //   __EDITOR_PHYS_MODE_H__
