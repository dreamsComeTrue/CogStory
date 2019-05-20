// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

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

        void ShowUI ();
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
        void RenderUI ();

        void ClearSelection ();
        
        bool IsVisible ();

        void SetDragging (bool drag);

    private:
        void EditTriggerArea ();

    private:
        Editor* m_Editor;

        bool m_IsVisible;
        
        TriggerArea* m_TriggerArea;
        Point* m_TriggerPoint;
        std::string m_TriggerAreaName;
        bool m_Collidable;

        char m_TriggerAreaWindow[100];
        char m_TriggerAreaData[100];

        bool m_Editing;
        bool m_Dragging;
    };
}

#endif //   __EDITOR_TRIGGER_AREA_MODE_H__
