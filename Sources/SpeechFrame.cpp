// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "SpeechFrame.h"
#include "Atlas.h"
#include "AtlasManager.h"
#include "Font.h"
#include "MainLoop.h"
#include "SceneManager.h"
#include "Screen.h"
#include "SpeechFrameManager.h"
#include <ctime>

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    const int TEXT_INSETS = 10;

    //--------------------------------------------------------------------------------------------------

    SpeechFrame::SpeechFrame (SpeechFrameManager* manager)
      : m_Manager (manager)
      , m_Visible (true)
      , m_DrawTextCenter (false)
      , m_DrawSpeed (10)
      , m_ArrowDrawSpeed (300)
      , m_DrawLightArrow (true)
      , m_CurrentDrawTime (0)
      , m_CurrentIndex (0)
      , m_CurrentLine (0)
      , m_DisplayLine (0)
      , m_StillUpdating (true)
      , m_MaxKeyDelta (200)
      , m_KeyEventHandled (false)
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
      , m_DrawSpeed (20)
      , m_ArrowDrawSpeed (300)
      , m_DrawLightArrow (true)
      , m_CurrentDrawTime (0)
      , m_CurrentIndex (0)
      , m_CurrentLine (0)
      , m_DisplayLine (0)
      , m_StillUpdating (true)
      , m_MaxKeyDelta (200)
      , m_KeyEventHandled (false)
    {
        m_FrameBitmap = load_nine_patch_bitmap (GetResourcePath (ResourceID::GFX_TEXT_FRAME).c_str ());
        SetText (text);
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
        if (m_KeyEventHandled)
        {
            m_KeyEventHandled = false;
            m_KeyDelta = 0.0f;
        }
        else
        {
            m_KeyDelta += deltaTime * 1000;
        }

        if (m_KeyDelta > m_MaxKeyDelta)
        {
            ALLEGRO_KEYBOARD_STATE state;
            al_get_keyboard_state (&state);

            if (al_key_down (&state, ALLEGRO_KEY_DOWN) || al_key_down (&state, ALLEGRO_KEY_S))
            {
                ++m_DisplayLine;
            }

            if (al_key_down (&state, ALLEGRO_KEY_UP) || al_key_down (&state, ALLEGRO_KEY_W))
            {
                --m_DisplayLine;
            }

            m_KeyDelta = 0.0f;
        }

        m_CurrentFlashTime += deltaTime * 1000;

        if (m_CurrentFlashTime >= m_ArrowDrawSpeed)
        {
            m_CurrentFlashTime = 0;
            m_DrawLightArrow = !m_DrawLightArrow;
        }

        if (m_StillUpdating)
        {
            m_CurrentDrawTime += deltaTime * 1000;

            if (m_CurrentDrawTime >= m_DrawSpeed)
            {
                m_CurrentDrawTime = 0.0f;
                ++m_CurrentIndex;

                if (m_CurrentIndex > m_TextLines[m_CurrentLine].size () - 1)
                {
                    ++m_CurrentLine;
                    m_CurrentIndex = 0;
                }

                if (m_CurrentLine >= m_TextLines.size ())
                {
                    m_StillUpdating = false;
                    m_CurrentLine = m_TextLines.size () - 1;
                    m_CurrentIndex = m_TextLines[m_CurrentLine].size () - 1;
                }
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechFrame::ProcessEvent (ALLEGRO_EVENT* event, float deltaTime)
    {
        if (event->type == ALLEGRO_EVENT_KEY_DOWN)
        {
            switch (event->keyboard.keycode)
            {
                case ALLEGRO_KEY_UP:
                {
                    --m_DisplayLine;
                    m_KeyEventHandled = true;

                    break;
                }
                case ALLEGRO_KEY_DOWN:
                {
                    ++m_DisplayLine;
                    m_KeyEventHandled = true;

                    break;
                }
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechFrame::SetText (const std::string& text)
    {
        m_Text = text;
        m_LineHeight =
          m_Manager->GetSceneManager ()->GetMainLoop ()->GetScreen ()->GetFont ().GetTextDimensions (FONT_NAME_MAIN_MEDIUM, text).Height;

        m_TextLines = SplitString (text, '\n');

        for (int i = 0; i < m_TextLines.size (); ++i)
        {
            std::vector<std::string> lines = BreakLine (TrimString (m_TextLines[i]), m_DrawRect.Transform.Size.Width - 2 * TEXT_INSETS);

            if (lines.size () > 1)
            {
                m_TextLines.erase (m_TextLines.begin () + i);
                m_TextLines.insert (m_TextLines.begin () + i, lines.begin (), lines.end ());
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechFrame::Render (float deltaTime)
    {
        if (m_Visible)
        {
            draw_nine_patch_bitmap (m_FrameBitmap,
                                    m_DrawRect.Transform.Pos.X,
                                    m_DrawRect.Transform.Pos.Y,
                                    m_DrawRect.Transform.Size.Width,
                                    m_DrawRect.Transform.Size.Height);

            int xPoint;
            int yPoint;
            int align;
            Font& font = m_Manager->GetSceneManager ()->GetMainLoop ()->GetScreen ()->GetFont ();
            std::string text = m_Text.substr (0, m_CurrentIndex);

            if (m_DrawTextCenter)
            {
                Point textDimensions = font.GetTextDimensions (FONT_NAME_MAIN_MEDIUM, text);

                xPoint = m_DrawRect.Transform.Pos.X + m_DrawRect.Transform.Size.Width * 0.5;
                yPoint = m_DrawRect.Transform.Pos.Y + m_DrawRect.Transform.Size.Height * 0.5 - textDimensions.Height * 0.5;
                align = ALLEGRO_ALIGN_CENTER;
            }
            else
            {
                xPoint = m_DrawRect.Transform.Pos.X + TEXT_INSETS;
                yPoint = m_DrawRect.Transform.Pos.Y + TEXT_INSETS;
                align = ALLEGRO_ALIGN_LEFT;
            }

            int maxLines = (m_DrawRect.Transform.Size.Height - 2 * TEXT_INSETS) / m_LineHeight;
            int margin = 5;
            int lineCounter = 0;
            int diff = m_CurrentLine + 1 - maxLines;

            if (m_CurrentLine + 1 >= maxLines)
            {
                lineCounter = diff;
            }

            lineCounter += m_DisplayLine;

            if (lineCounter < 0)
            {
                lineCounter = 0;
                m_DisplayLine = -diff;
            }

            if (diff >= 0 && lineCounter > diff)
            {
                lineCounter = diff;
                m_DisplayLine = 0;
            }

            int xOffset = 20;
            int yOffset = 2;
            std::string regionName = m_DrawLightArrow ? "arrow_light" : "arrow_dark";

            //  Up arrow
            if (lineCounter > 0)
            {
                m_Manager->GetSceneManager ()
                  ->GetAtlasManager ()
                  ->GetAtlas (GetBaseName (GetResourcePath (PACK_MENU_UI)))
                  ->DrawRegion (regionName,
                                m_DrawRect.Transform.Pos.X + m_DrawRect.Transform.Size.Width - xOffset,
                                m_DrawRect.Transform.Pos.Y + yOffset,
                                1.0f,
                                1.0f,
                                DegressToRadians (180.0f));
            }

            //  Down arrow
            if (lineCounter < diff)
            {
                m_Manager->GetSceneManager ()
                  ->GetAtlasManager ()
                  ->GetAtlas (GetBaseName (GetResourcePath (PACK_MENU_UI)))
                  ->DrawRegion (regionName,
                                m_DrawRect.Transform.Pos.X + m_DrawRect.Transform.Size.Width - xOffset - 10,
                                m_DrawRect.Transform.Pos.Y + m_DrawRect.Transform.Size.Height - yOffset - 6);
            }

            int x, y, w, h;
            al_get_clipping_rectangle (&x, &y, &w, &h);
            al_set_clipping_rectangle (
              m_DrawRect.Transform.Pos.X, m_DrawRect.Transform.Pos.Y, m_DrawRect.Transform.Size.Width, m_DrawRect.Transform.Size.Height);

            for (int i = 0; lineCounter < m_CurrentLine + 1; ++lineCounter, ++i)
            {
                if (lineCounter > m_TextLines.size () - 1)
                {
                    break;
                }

                std::string t = m_TextLines[lineCounter];

                if (lineCounter >= m_CurrentLine)
                {
                    t = t.substr (0, m_CurrentIndex);
                }

                int y = yPoint + i * (m_LineHeight + margin);

                font.DrawText (FONT_NAME_MAIN_MEDIUM, COLOR_WHITE, xPoint, y, t, align);
            }

            al_set_clipping_rectangle (x, y, w, h);
        }
    }

    //--------------------------------------------------------------------------------------------------

    std::vector<std::string> SpeechFrame::BreakLine (const std::string& line, float maxWidth)
    {
        std::vector<std::string> ret;

        float width =
          m_Manager->GetSceneManager ()->GetMainLoop ()->GetScreen ()->GetFont ().GetTextDimensions (FONT_NAME_MAIN_MEDIUM, line).Width;

        if (width >= maxWidth)
        {
            std::string workLine = line;
            int length = 0;

            while (length < workLine.size () - 2)
            {
                std::string currentPart = workLine.substr (0, length + 1);

                width = m_Manager->GetSceneManager ()
                          ->GetMainLoop ()
                          ->GetScreen ()
                          ->GetFont ()
                          .GetTextDimensions (FONT_NAME_MAIN_MEDIUM, currentPart)
                          .Width;

                if (width >= maxWidth)
                {
                    length = currentPart.rfind (' ');

                    std::string str = workLine.substr (0, length);
                    ret.push_back (TrimString (str));
                    workLine = workLine.substr (length);
                }
                else
                {
                    ++length;
                }
            }

            if (workLine != "")
            {
                ret.push_back (TrimString (workLine));
            }
        }

        return ret;
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechFrame::SetDrawRect (Rect rect) { m_DrawRect = rect; }

    //--------------------------------------------------------------------------------------------------

    void SpeechFrame::SetVisible (bool visible)
    {
        m_Visible = visible;
        m_CurrentIndex = 0;
        m_CurrentLine = 0;
        m_CurrentDrawTime = 0;
        m_DisplayLine = 0;
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
