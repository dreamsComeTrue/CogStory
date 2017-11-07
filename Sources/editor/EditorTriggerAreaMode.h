// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_TRIGGER_AREA_MODE_H__
#define __EDITOR_TRIGGER_AREA_MODE_H__

#include "Common.h"
#include "Scene.h"

namespace aga
{
    class Editor;

    class EditorTriggerAreaMode
    {
        friend class Editor;

    public:
        EditorTriggerAreaMode (Editor* editor);
        virtual ~EditorTriggerAreaMode ();

    private:
        bool MoveSelectedTriggerPoint ();
        void DrawTriggerAreas (float mouseX, float mouseY);

        Point* GetTriggerPointUnderCursor (int mouseX, int mouseY);
        TriggerArea* GetTriggerAreaUnderCursor (int mouseX, int mouseY);
        void InsertTriggerAreaAtCursor (int mouseX, int mouseY);
        bool RemoveTriggerPointUnderCursor (int mouseX, int mouseY);

        void NewTriggerArea ();

    private:
        Editor* m_Editor;

        TriggerArea* m_TriggerArea;
        Point* m_TriggerPoint;
        char m_TriggerAreaName[64];
    };
}

#endif //   __EDITOR_TRIGGER_AREA_MODE_H__
