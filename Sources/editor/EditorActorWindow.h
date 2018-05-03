// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_ACTOR_WINDOW_H__
#define __EDITOR_ACTOR_WINDOW_H__

#include "Common.h"
#include "Entity.h"

#include <Gwork/Controls.h>

namespace aga
{
    class Editor;

    class EditorActorWindow : public Gwk::Event::Handler
    {
    public:
        EditorActorWindow (Editor* editor, Gwk::Controls::Canvas* canvas);
        virtual ~EditorActorWindow ();

        void Show ();
        void SelectActor (const std::string& name);

        Gwk::Controls::WindowControl* GetSceneWindow () { return m_SceneWindow; }

        void RenderActorImage ();

    private:
        void OnSave ();
        void OnRemove ();

        void OnAddScript ();
        void OnRemoveScript (Gwk::Controls::Base* control);

        void OnAddComponent ();
        void OnRemoveComponent (Gwk::Controls::Base* control);

        void OnAccept ();
        void OnCancel ();

        void OnImagePathSelected (Gwk::Controls::Base* control);
        void OnImageSelected (Gwk::Controls::Base* control);

        void UpdateActorsTree ();
        void OnActorSelect (Gwk::Controls::Base* control);

        void AddScriptEntry (const std::string& name, const std::string& path);
        void AddComponentEntry (const std::string& name, const std::string& type);

        std::string GetImageName (class Actor* actor);
        std::string GetImagePath (class Actor* actor);

        Gwk::Controls::TreeNode* FindNode (Gwk::Controls::TreeNode* node, const std::string& name);

    private:
        Editor* m_Editor;

        int m_SelectedType;
        float m_Rotation;
        int m_ZOrder;
        Point m_Position;

        class Actor* m_SelectedActor;

        Gwk::Controls::WindowControl* m_SceneWindow;
        Gwk::Controls::TreeControl* m_ActorsTree;

        Gwk::Controls::Property::ComboBox* m_ImageComboBox;
        Gwk::Controls::Property::ComboBox* m_ImagePathComboBox;

        Gwk::Controls::PropertyTree* m_ActorProperties;
        Gwk::Controls::Properties* m_GeneralSection;
        Gwk::Controls::Properties* m_TransformSection;
        Gwk::Controls::Properties* m_ApperanceSection;
        Gwk::Controls::Properties* m_ScriptSection;
        Gwk::Controls::Properties* m_ComponentSection;

        class Atlas* m_SelectedAtlas;
        std::string m_SelectedAtlasRegion;
    };
}

#endif //   __EDITOR_ACTOR_WINDOW_H__
