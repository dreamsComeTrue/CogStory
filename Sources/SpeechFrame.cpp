// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "SpeechFrame.h"
#include "Font.h"
#include "MainLoop.h"
#include "SceneManager.h"
#include "Screen.h"
#include "SpeechFrameManager.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    const int TEXT_INSETS = 10;

    //--------------------------------------------------------------------------------------------------

    SpeechFrame::SpeechFrame (SpeechFrameManager* manager)
        : m_Manager (manager)
        , m_Visible (true)
        , m_DrawTextCenter (false)
        , m_DrawSpeed (40)
        , m_CurrentDrawTime (0)
        , m_CurrentIndex (0)
        , m_StillUpdating (true)
    {
        m_FrameBitmap = load_nine_patch_bitmap (GetResourcePath (ResourceID::GFX_TEXT_FRAME).c_str ());
    }

    //--------------------------------------------------------------------------------------------------

    SpeechFrame::SpeechFrame (SpeechFrameManager* manager, const std::string& text, Rect rect)
        : m_Manager (manager)
        , m_Text (text)
        , m_DrawRect (rect)
        , m_Visible (true)
        , m_DrawTextCenter (false)
        , m_DrawSpeed (40)
        , m_CurrentDrawTime (0)
        , m_CurrentIndex (0)
        , m_StillUpdating (true)
    {
        m_FrameBitmap = load_nine_patch_bitmap (GetResourcePath (ResourceID::GFX_TEXT_FRAME).c_str ());
    }

    //--------------------------------------------------------------------------------------------------

    SpeechFrame::~SpeechFrame ()
    {
        if (m_FrameBitmap)
        {
            destroy_nine_patch_bitmap (m_FrameBitmap);
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool SpeechFrame::Update (float deltaTime)
    {
        if (m_StillUpdating)
        {
            m_CurrentDrawTime += deltaTime * 1000;

            if (m_CurrentDrawTime >= m_DrawSpeed)
            {
                m_CurrentDrawTime = 0;
                ++m_CurrentIndex;

                if (m_CurrentIndex > m_Text.size () - 1)
                {
                    m_CurrentIndex == m_Text.size () - 1;
                    m_StillUpdating = false;
                }
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechFrame::Render (float deltaTime)
    {
        if (m_Visible)
        {
            draw_nine_patch_bitmap (m_FrameBitmap, m_DrawRect.Transform.Pos.X, m_DrawRect.Transform.Pos.Y,
                m_DrawRect.Transform.Size.Width, m_DrawRect.Transform.Size.Height);

            int xPoint;
            int yPoint;
            int align;
            Font& font = m_Manager->GetSceneManager ()->GetMainLoop ()->GetScreen ()->GetFont ();
            std::string text = m_Text.substr (0, m_CurrentIndex);

            if (m_DrawTextCenter)
            {
                Point textDimensions = font.GetTextDimensions (FONT_NAME_MAIN_MEDIUM, text);

                xPoint = m_DrawRect.Transform.Pos.X + m_DrawRect.Transform.Size.Width * 0.5;
                yPoint
                    = m_DrawRect.Transform.Pos.Y + m_DrawRect.Transform.Size.Height * 0.5 - textDimensions.Height * 0.5;
                align = ALLEGRO_ALIGN_CENTER;
            }
            else
            {
                xPoint = m_DrawRect.Transform.Pos.X + TEXT_INSETS;
                yPoint = m_DrawRect.Transform.Pos.Y + TEXT_INSETS;
                align = ALLEGRO_ALIGN_LEFT;
            }

            font.DrawText (FONT_NAME_MAIN_MEDIUM, COLOR_WHITE, xPoint, yPoint, text, align);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechFrame::SetDrawRect (Rect rect) { m_DrawRect = rect; }

    //--------------------------------------------------------------------------------------------------

    void SpeechFrame::SetVisible (bool visible)
    {
        m_Visible = visible;
        m_CurrentIndex = 0;
        m_CurrentDrawTime = 0;
        m_StillUpdating = true;
    }

    //--------------------------------------------------------------------------------------------------

    bool SpeechFrame::IsVisible () const { return m_Visible; }

    //--------------------------------------------------------------------------------------------------

    void SpeechFrame::SetDrawTextCenter (bool center) { m_DrawTextCenter = center; }

    //--------------------------------------------------------------------------------------------------

    bool SpeechFrame::IsDrawTextCenter () const { return m_DrawTextCenter; }

    //--------------------------------------------------------------------------------------------------
}
