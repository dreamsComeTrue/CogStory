// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_ACTOR_WINDOW_H__
#define __EDITOR_ACTOR_WINDOW_H__

#include "Common.h"
#include "Entity.h"

#include <Gwork/Controls.h>

namespace aga
{
    class Editor;

    class EditorActorScriptWindow : public Gwk::Event::Handler
    {
    public:
        EditorActorScriptWindow (Editor* editor, Gwk::Controls::Canvas* canvas);
        virtual ~EditorActorScriptWindow ();

        void Show (std::function<bool(void)> OnAcceptFunc, std::function<bool(void)> OnCancelFunc);
        bool GetResult ();

    private:
        void OnBrowse ();
        void OnAccept ();
        void OnCancel ();

        void OnNameEdit (Gwk::Controls::Base* control);
        void OnPathEdit (Gwk::Controls::Base* control);

    public:
        std::string Name;
        std::string Path;

    private:
        Editor* m_Editor;
        Gwk::Controls::WindowControl* m_SceneWindow;
        bool m_Result;

        std::function<bool(void)> m_OnAcceptFunc;
        std::function<bool(void)> m_OnCancelFunc;
    };

    class EditorActorWindow : public Gwk::Event::Handler
    {
    public:
        EditorActorWindow (Editor* editor, Gwk::Controls::Canvas* canvas);
        virtual ~EditorActorWindow ();

        void Show ();
        void SelectActor (const std::string& name);

    private:
        void OnSave ();
        void OnRemove ();

        void OnAddScript ();
        void OnRemoveScript (Gwk::Controls::Base* control);

        void OnCollision ();

        void OnAccept ();
        void OnCancel ();

        void UpdateActorsTree ();
        void OnActorSelect (Gwk::Controls::Base* control);

    private:
        Editor* m_Editor;

        int m_SelectedType;
        float m_Rotation;
        Point m_Position;

        Gwk::Controls::WindowControl* m_SceneWindow;
        Gwk::Controls::TreeControl* m_ActorsTree;

        Gwk::Controls::PropertyTree* m_ActorProperties;
        Gwk::Controls::Properties* m_GeneralSection;
        Gwk::Controls::Properties* m_TransformSection;
        Gwk::Controls::Properties* m_ScriptSection;

        EditorActorScriptWindow* m_ScriptWindow;
    };
}

#endif //   __EDITOR_ACTOR_WINDOW_H__
