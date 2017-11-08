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
    {
        m_FrameBitmap = load_nine_patch_bitmap (GetResourcePath (ResourceID::GFX_TEXT_FRAME).c_str ());
    }

    //--------------------------------------------------------------------------------------------------

    SpeechFrame::SpeechFrame (SpeechFrameManager* manager, const std::string& text, Rect rect)
        : m_Manager (manager)
        , m_Text (text)
        , m_DrawRect (rect)
        , m_Visible (true)
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

    bool SpeechFrame::Update (float deltaTime) {}

    //--------------------------------------------------------------------------------------------------

    void SpeechFrame::Render (float deltaTime)
    {
        if (m_Visible)
        {
            draw_nine_patch_bitmap (m_FrameBitmap, m_DrawRect.Transform.Pos.X, m_DrawRect.Transform.Pos.Y,
                m_DrawRect.Transform.Size.Width, m_DrawRect.Transform.Size.Height);

            int xPoint = m_DrawRect.Transform.Pos.X + TEXT_INSETS;
            int yPoint = m_DrawRect.Transform.Pos.Y + TEXT_INSETS;
            m_Manager->GetSceneManager ()->GetMainLoop ()->GetScreen ()->GetFont ().DrawText (
                FONT_NAME_MAIN_MEDIUM, COLOR_WHITE, xPoint, yPoint, m_Text, ALLEGRO_ALIGN_LEFT);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechFrame::SetDrawRect (Rect rect) { m_DrawRect = rect; }

    //--------------------------------------------------------------------------------------------------

    void SpeechFrame::SetVisible (bool visible) { m_Visible = visible; }

    //--------------------------------------------------------------------------------------------------

    bool SpeechFrame::IsVisible () const { return m_Visible; }

    //--------------------------------------------------------------------------------------------------
}
