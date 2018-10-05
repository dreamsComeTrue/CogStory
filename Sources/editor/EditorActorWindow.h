// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_ACTOR_WINDOW_H__
#define __EDITOR_ACTOR_WINDOW_H__

#include "Common.h"
#include "Entity.h"

namespace aga
{
    class Editor;

    class EditorActorWindow
    {
    public:
        EditorActorWindow (Editor* editor);
        virtual ~EditorActorWindow ();

        void Show ();
        void SelectActor (class Actor* actor);

        void RenderUI ();
        bool IsVisible () const { return m_IsVisible; }

        void RenderActorImage ();

    private:
        void OnSave ();

        void OnAddScript ();

        void OnAddComponent ();
        void OnRemoveComponent ();

        void UpdateImageCombos ();

        void UpdateComboBoxes ();
        void OnActorSelect (int id);

        void AddScriptEntry (const std::string& name, const std::string& path);
        void AddComponentEntry (const std::string& name, const std::string& type);

        void FillComponentsList ();

    private:
        Editor* m_Editor;
        bool m_IsVisible;

        class Actor* m_SelectedActor;

        class Atlas* m_SelectedAtlas;
        std::string m_SelectedAtlasRegion;

        char m_ActorID[100];
        char m_ActorName[100];

        char m_ActorPosition[100];
        float m_ActorRotation;
        int m_ActorZOrder;

        char m_ActorAnimation[100];
        float m_ActorFocusHeight;
        bool m_ActorCollidable;
        bool m_ActorCollision;
        bool m_ActorOverlap;

        std::vector<std::string> m_ActorTypes;
        int m_SelectedActorType;

        std::vector<std::string> m_Blueprints;
        int m_SelectedBlueprint;

        std::vector<std::string> m_Images;
        int m_SelectedImage;

        std::vector<std::string> m_ImagePaths;
        int m_SelectedImagePath;

        std::vector<std::string> m_Animations;
        int m_SelectedAnimation;
    };
}

#endif //   __EDITOR_ACTOR_WINDOW_H__
