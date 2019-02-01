// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_ANIMATION_WINDOW_H__
#define __EDITOR_ANIMATION_WINDOW_H__

#include "Animation.h"
#include "Common.h"

namespace aga
{
    class Editor;

    class EditorAnimationWindow
    {
    public:
        EditorAnimationWindow (Editor* editor);
        virtual ~EditorAnimationWindow ();

        void Show (std::function<void(std::string, std::string)> OnAcceptFunc,
            std::function<void(std::string, std::string)> OnCancelFunc);

        void Render ();
        void RenderSpritesheet ();
        void RenderAnimationFrames (float deltaTime);

        void SelectAnimationFrame ();

        bool IsVisible ();

    private:
        void OnSave ();
        void OnAccept ();
        void OnCancel ();

        void UpdateImageCombos ();
        void UpdateAnimations ();
        void UpdateNames ();

    private:
        Editor* m_Editor;

        class Atlas* m_SelectedAtlas;
        std::string m_SelectedAtlasRegion;

        char m_Animation[100];
        char m_Name[100];
        std::vector<std::string> m_Animations;
        int m_SelectedAnimation;

        std::vector<std::string> m_Names;
        int m_SelectedName;

        std::vector<std::string> m_Images;
        int m_SelectedImage;

        std::vector<std::string> m_ImagePaths;
        int m_SelectedImagePath;

        int m_AnimSpeed;

        bool m_IsVisible;

        class AtlasRegion* m_HoveredRegion;
        Rect m_HoveredArea;

        std::function<void(std::string, std::string)> m_OnAcceptFunc;
        std::function<void(std::string, std::string)> m_OnCancelFunc;

        Animation m_AnimationData;
        std::vector<AnimationFrameEntry> m_Frames;
        int m_CurrentFrameIndex;
        float m_FrameTimeLeft;

        int m_CellX, m_CellY;
    };
}

#endif //   __EDITOR_ANIMATION_WINDOW_H__
