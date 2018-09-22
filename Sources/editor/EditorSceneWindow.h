// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_SCENE_WINDOW_H__
#define __EDITOR_SCENE_WINDOW_H__

#include "Common.h"
#include "Entity.h"

#include <Gwork/Controls.h>

#include "imgui.h"

namespace aga
{
    class Editor;

    class EditorSceneWindow : public Gwk::Event::Handler
    {
    public:
        EditorSceneWindow (Editor* editor, Gwk::Controls::Canvas* canvas);
        virtual ~EditorSceneWindow ();

        void Show ();
        void UpdateScriptsBox ();

        void RenderUI ();

        Gwk::Controls::WindowControl* GetSceneWindow () { return m_SceneWindow; }

    private:
        void OnAddScript ();
        void OnReloadScript (Gwk::Controls::Base* control);
        void OnRemoveScript (Gwk::Controls::Base* control);

        void OnNameEdit ();
        void OnScriptSelected (Gwk::Controls::Base* control);

        Gwk::Controls::Property::LabelTwoButton* GetScriptEntryByControl (Gwk::Controls::Base* control);

    private:
        Editor* m_Editor;

        Gwk::Controls::WindowControl* m_SceneWindow;
        Gwk::Controls::TextBox* m_NameTextBox;
        Gwk::Controls::PropertyTree* m_ScriptsBox;
        Gwk::Controls::ColorPicker* m_ColorPicker;

        Gwk::Controls::Properties* m_ScriptSection;
        Gwk::Controls::TreeNode* m_SelectedNode;

        char m_SceneName[100];
        ImVec4 m_BackColor;
    };
}

#endif //   __EDITOR_SCENE_WINDOW_H__
