// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_TRIGGER_AREA_MODE_H__
#define __EDITOR_TRIGGER_AREA_MODE_H__

#include "Common.h"
#include "Scene.h"

namespace aga
{
    class Editor;
    class TriggerAreaWindow;

    class EditorTriggerAreaMode
    {
    public:
        EditorTriggerAreaMode (Editor* editor);
        virtual ~EditorTriggerAreaMode ();

        bool MoveSelectedTriggerPoint ();
        void DrawTriggerAreas (float mouseX, float mouseY);

        Point* GetTriggerPointUnderCursor (int mouseX, int mouseY);
        TriggerArea* GetTriggerAreaUnderCursor (int mouseX, int mouseY);
        void InsertTriggerAreaAtCursor (int mouseX, int mouseY);
        bool RemoveTriggerPointUnderCursor (int mouseX, int mouseY);
        void NewTriggerArea ();

        Point* GetTriggerPoint () { return m_TriggerPoint; }
        void SetTriggerPoint (Point* point) { m_TriggerPoint = point; }

        TriggerArea* GetTriggerArea () { return m_TriggerArea; }
        void SetTriggerArea (TriggerArea* area) { m_TriggerArea = area; }

        std::string GetTriggerAreaName () { return m_TriggerAreaName; }
        void SetTriggerAreaName (const std::string& name) { m_TriggerAreaName = name; }

    private:
        Editor* m_Editor;

        TriggerArea* m_TriggerArea;
        Point* m_TriggerPoint;
        std::string m_TriggerAreaName;
    };
}

#endif //   __EDITOR_TRIGGER_AREA_MODE_H__
