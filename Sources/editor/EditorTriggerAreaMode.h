// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_TRIGGER_AREA_MODE_H__
#define __EDITOR_TRIGGER_AREA_MODE_H__

#include "Common.h"

namespace aga
{
    class Editor;
    class TriggerAreaWindow;
    struct TriggerArea;

    class EditorTriggerAreaMode
    {
    public:
        EditorTriggerAreaMode (Editor* editor);
        virtual ~EditorTriggerAreaMode ();

        bool MoveSelectedTriggerPoint ();
        void MarkSelectedTriggerAreas ();

        Point* GetTriggerPointUnderCursor (int mouseX, int mouseY);
        TriggerArea* GetTriggerAreaUnderCursor (int mouseX, int mouseY);
        void InsertTriggerAreaAtCursor (int mouseX, int mouseY);
        bool RemoveTriggerPointUnderCursor (int mouseX, int mouseY);
        void NewTriggerArea ();

        Point* GetTriggerPoint ();
        void SetTriggerPoint (Point* point);

        TriggerArea* GetTriggerArea ();
        void SetTriggerArea (TriggerArea* area);

        std::string GetTriggerAreaName ();
        void SetTriggerAreaName (const std::string& name);

        void SetTriggerAreaCollidable (bool collidable);
        bool IsTriggerAreaCollidable () const;

        void Render ();

        void ClearSelection ();

    private:
        Editor* m_Editor;

        TriggerArea* m_TriggerArea;
        Point* m_TriggerPoint;
        std::string m_TriggerAreaName;
        bool m_Collidable;

        char m_TriggerAreaWindow[100];
        char m_TriggerAreaData[100];

        bool m_Editing;
    };
}

#endif //   __EDITOR_TRIGGER_AREA_MODE_H__
