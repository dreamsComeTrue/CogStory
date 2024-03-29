// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_ACTOR_WINDOW_H__
#define __EDITOR_ACTOR_WINDOW_H__

#include "Common.h"

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
        bool IsVisible () const;

        void RenderActorImage ();

        void UpdateSpeeches ();

    private:
        void OnSave ();
        void OnRemove ();

        void OnAddScript ();

        void OnAddComponent ();
        void OnRemoveComponent ();

        void UpdateImageCombos ();

        void UpdateComboBoxes ();
        void OnActorSelect (int id);

        void AddScriptEntry (const std::string& name, const std::string& path);
        void AddComponentEntry (const std::string& name, const std::string& type);

        void FillComponentsList ();
        
        void RenderActorGroup (const std::string& groupName);

    private:
        Editor* m_Editor;
        bool m_IsVisible;

        class Actor* m_SelectedActor;

        class Atlas* m_SelectedAtlas;
        std::string m_SelectedAtlasRegion;

        char m_ActorID[100];
        char m_ActorName[100];

        char m_ActorPosition[100];
        char m_ActorRotation[20];
        int m_ActorZOrder;

        char m_ActorAnimation[100];
        char m_ActorFocusHeight[20];
        bool m_ActorCollidable;
        bool m_ActorCollision;
        char m_ActorOverlapSize[100];
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

        std::vector<std::string> m_Speeches;
        int m_SelectedSpeech;
        bool m_ActionSpeechHandling;
    };
}

#endif //   __EDITOR_ACTOR_WINDOW_H__
