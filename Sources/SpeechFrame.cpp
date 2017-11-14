// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "SpeechFrame.h"
#include "Atlas.h"
#include "AtlasManager.h"
#include "Font.h"
#include "MainLoop.h"
#include "SceneManager.h"
#include "Screen.h"
#include "SpeechFrameManager.h"

#include <stack>

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
      , m_ScrollPossible (false)
      , m_ShouldBeHandled (true)
      , m_Handled (false)
    {
        m_FrameBitmap = load_nine_patch_bitmap (GetResourcePath (ResourceID::GFX_TEXT_FRAME).c_str ());
        m_Atlas = m_Manager->GetSceneManager ()->GetAtlasManager ()->GetAtlas (GetBaseName (GetResourcePath (PACK_CHARACTERS_UI)));
    }

    //--------------------------------------------------------------------------------------------------

    SpeechFrame::SpeechFrame (SpeechFrameManager* manager,
                              const std::string& text,
                              Rect rect,
                              bool shouldBeHandled,
                              const std::string& regionName)
      : m_Manager (manager)
      , m_DrawRect (rect)
      , m_Visible (true)
      , m_DrawTextCenter (false)
      , m_DrawSpeed (11)
      , m_ArrowDrawSpeed (300)
      , m_DrawLightArrow (true)
      , m_CurrentDrawTime (0)
      , m_CurrentIndex (0)
      , m_CurrentLine (0)
      , m_DisplayLine (0)
      , m_StillUpdating (true)
      , m_MaxKeyDelta (200)
      , m_KeyEventHandled (false)
      , m_ScrollPossible (false)
      , m_ShouldBeHandled (shouldBeHandled)
      , m_Handled (false)
      , m_RegionName (regionName)
    {
        m_FrameBitmap = load_nine_patch_bitmap (GetResourcePath (ResourceID::GFX_TEXT_FRAME).c_str ());

        if (m_RegionName != "")
        {
            m_Atlas = m_Manager->GetSceneManager ()->GetAtlasManager ()->GetAtlas (GetBaseName (GetResourcePath (PACK_CHARACTERS_UI)));
        }

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

            int lineCounter = GetLineCounter ();

            if (al_key_down (&state, ALLEGRO_KEY_UP) || al_key_down (&state, ALLEGRO_KEY_W))
            {
                --m_DisplayLine;

                if (lineCounter > 0)
                {
                    if (ScrollUpFunction)
                    {
                        ScrollUpFunction ();
                    }
                }
            }

            if (al_key_down (&state, ALLEGRO_KEY_DOWN) || al_key_down (&state, ALLEGRO_KEY_S))
            {
                ++m_DisplayLine;

                int maxLines = (m_DrawRect.GetSize ().Height - 2 * TEXT_INSETS) / m_LineHeight;
                int diff = m_CurrentLine + 1 - maxLines;

                if (lineCounter < diff)
                {
                    if (ScrollDownFunction)
                    {
                        ScrollDownFunction ();
                    }
                }
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
                    m_CurrentIndex = m_TextLines[m_CurrentLine].size ();
                }
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechFrame::ProcessEvent (ALLEGRO_EVENT* event, float deltaTime)
    {
        if (event->type == ALLEGRO_EVENT_KEY_DOWN)
        {
            int lineCounter = GetLineCounter ();

            switch (event->keyboard.keycode)
            {
                case ALLEGRO_KEY_UP:
                {
                    --m_DisplayLine;

                    if (lineCounter > 0)
                    {
                        if (ScrollUpFunction)
                        {
                            ScrollUpFunction ();
                        }
                    }

                    m_KeyEventHandled = true;

                    break;
                }

                case ALLEGRO_KEY_DOWN:
                {
                    ++m_DisplayLine;

                    int maxLines = (m_DrawRect.GetSize ().Height - 2 * TEXT_INSETS) / m_LineHeight;
                    int diff = m_CurrentLine + 1 - maxLines;

                    if (lineCounter < diff)
                    {
                        if (ScrollDownFunction)
                        {
                            ScrollDownFunction ();
                        }
                    }

                    m_KeyEventHandled = true;

                    break;
                }

                case ALLEGRO_KEY_ENTER:
                {
                    m_Handled = true;

                    if (HandledFunction)
                    {
                        HandledFunction ();
                    }

                    break;
                }
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    int SpeechFrame::GetLineCounter ()
    {
        int maxLines = (m_DrawRect.GetSize ().Height - 2 * TEXT_INSETS) / m_LineHeight;
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
        }

        if (diff >= 0 && lineCounter > diff)
        {
            lineCounter = diff;
        }

        return lineCounter;
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechFrame::SetText (const std::string& text)
    {
        m_Text = text;
        m_Attributes.clear ();
        PreprocessText (m_Text);

        m_LineHeight =
          m_Manager->GetSceneManager ()->GetMainLoop ()->GetScreen ()->GetFont ().GetTextDimensions (FONT_NAME_MAIN_MEDIUM, m_Text).Height;

        m_TextLines = BreakLine (m_Text, m_DrawRect.GetSize ().Width - 2 * TEXT_INSETS);
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechFrame::Render (float deltaTime)
    {
        if (m_Visible)
        {
            if (m_Atlas && m_RegionName != "")
            {
                Point characterOffset = Point (20, 10);
                int edgeLength = m_DrawRect.GetSize ().Height - 2 * characterOffset.Y;
                AtlasRegion region = m_Atlas->GetRegion (m_RegionName);

                float ratio = std::min ((float)edgeLength / region.Bounds.Size.Width, (float)edgeLength / region.Bounds.Size.Height);

                m_Atlas->DrawRegion (m_RegionName,
                                     m_DrawRect.GetPos ().X - ratio * region.Bounds.Size.Width * 0.5f - characterOffset.X,
                                     m_DrawRect.GetPos ().Y + m_DrawRect.GetSize ().Y * 0.5f,
                                     ratio,
                                     ratio,
                                     0);
            }

            draw_nine_patch_bitmap (
              m_FrameBitmap, m_DrawRect.GetPos ().X, m_DrawRect.GetPos ().Y, m_DrawRect.GetSize ().Width, m_DrawRect.GetSize ().Height);

            int xPoint;
            int yPoint;
            int align;
            Font& font = m_Manager->GetSceneManager ()->GetMainLoop ()->GetScreen ()->GetFont ();
            std::string text = m_Text.substr (0, m_CurrentIndex);

            int centerOffset = 0;
            if (m_DrawTextCenter)
            {
                Point textDimensions = font.GetTextDimensions (FONT_NAME_MAIN_MEDIUM, text);

                xPoint = m_DrawRect.GetPos ().X + m_DrawRect.GetSize ().Width * 0.5;
                yPoint = m_DrawRect.GetPos ().Y + m_DrawRect.GetSize ().Height * 0.5 - textDimensions.Height * 0.5;
                align = ALLEGRO_ALIGN_CENTER;
                centerOffset = -textDimensions.Width * 0.5f + 2 * TEXT_INSETS;
            }
            else
            {
                xPoint = m_DrawRect.GetPos ().X + TEXT_INSETS;
                yPoint = m_DrawRect.GetPos ().Y + TEXT_INSETS;
                align = ALLEGRO_ALIGN_LEFT;
            }

            int maxLines = (m_DrawRect.GetSize ().Height - 2 * TEXT_INSETS) / m_LineHeight;
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
                                m_DrawRect.GetPos ().X + m_DrawRect.GetSize ().Width - xOffset,
                                m_DrawRect.GetPos ().Y + yOffset,
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
                                m_DrawRect.GetPos ().X + m_DrawRect.GetSize ().Width - xOffset - 10,
                                m_DrawRect.GetPos ().Y + m_DrawRect.GetSize ().Height - yOffset - 6);
            }

            int x, y, w, h;
            al_get_clipping_rectangle (&x, &y, &w, &h);
            al_set_clipping_rectangle (
              m_DrawRect.GetPos ().X, m_DrawRect.GetPos ().Y, m_DrawRect.GetSize ().Width, m_DrawRect.GetSize ().Height);

            for (int i = 0; lineCounter < m_CurrentLine + 1; ++lineCounter, ++i)
            {
                int currentCharIndex = 0;
                for (int j = 0; j < lineCounter; ++j)
                {
                    currentCharIndex += m_TextLines[j].size ();
                }

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

                float advance = 0;

                if (m_DrawTextCenter)
                {
                    advance = centerOffset;
                }

                for (int j = 0; j < t.length (); ++j)
                {
                    ALLEGRO_COLOR color = COLOR_WHITE;

                    for (int k = 0; k < m_Attributes.size (); ++k)
                    {
                        SpeechTextAttribute& attr = m_Attributes[k];

                        if (currentCharIndex >= attr.BeginIndex && currentCharIndex <= attr.EndIndex)
                        {
                            color = attr.Color;
                            break;
                        }
                    }

                    ++currentCharIndex;

                    std::string begin = t.substr (0, j);
                    if (!(isspace (t[j]) && TrimString (begin) == ""))
                    {
                        std::string textToDraw = std::string (1, t[j]);

                        font.DrawText (FONT_NAME_MAIN_MEDIUM, color, xPoint + advance, y, textToDraw, align);

                        if (textToDraw == " ")
                        {
                            advance += 10;
                        }
                        else
                        {
                            advance += m_Manager->GetSceneManager ()
                                         ->GetMainLoop ()
                                         ->GetScreen ()
                                         ->GetFont ()
                                         .GetTextDimensions (FONT_NAME_MAIN_MEDIUM, textToDraw)
                                         .Width;
                        }
                    }
                }
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
            int currentIndex = 0;

            while (currentIndex < workLine.size ())
            {
                std::string currentPart = workLine.substr (0, currentIndex + 1);

                if (currentPart[currentPart.length () - 1] == '\n')
                {
                    std::string str = workLine.substr (0, currentIndex);
                    ret.push_back (str);
                    workLine = workLine.substr (currentIndex + 1);
                }
                else
                {
                    width = m_Manager->GetSceneManager ()
                              ->GetMainLoop ()
                              ->GetScreen ()
                              ->GetFont ()
                              .GetTextDimensions (FONT_NAME_MAIN_MEDIUM, currentPart)
                              .Width;

                    if (width >= maxWidth)
                    {
                        currentIndex = currentPart.rfind (' ');

                        std::string str = workLine.substr (0, currentIndex);
                        ret.push_back (str);
                        workLine = workLine.substr (currentIndex);
                        currentIndex = 0;
                    }
                    else
                    {
                        ++currentIndex;
                    }
                }
            }

            if (workLine != "")
            {
                ret.push_back (workLine);
            }
        }
        else
        {
            std::string txt = std::string (line);
            ret.push_back (txt);
        }

        //  PreprocessText ("");

        return ret;
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechFrame::SetDrawRect (Rect rect) { m_DrawRect = rect; }

    //--------------------------------------------------------------------------------------------------

    void SpeechFrame::Show ()
    {
        m_Visible = true;
        m_CurrentIndex = 0;
        m_CurrentLine = 0;
        m_CurrentDrawTime = 0;
        m_DisplayLine = 0;
        m_ScrollPossible = false;
        m_StillUpdating = true;
        m_Handled = false;
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechFrame::Hide () { m_Visible = false; }

    //--------------------------------------------------------------------------------------------------

    bool SpeechFrame::IsVisible () const { return m_Visible; }

    //--------------------------------------------------------------------------------------------------

    void SpeechFrame::SetDrawTextCenter (bool center) { m_DrawTextCenter = center; }

    //--------------------------------------------------------------------------------------------------

    bool SpeechFrame::IsDrawTextCenter () const { return m_DrawTextCenter; }

    //--------------------------------------------------------------------------------------------------

    void SpeechFrame::PreprocessText (std::string& text)
    {
        std::stack<SpeechTextAttribute> attributes;
        int currIndex = 0;
        int newLinesCount = 0;

        while (currIndex < text.length ())
        {
            if (text[currIndex] == '\n')
            {
                ++newLinesCount;
            }
            else if (text[currIndex] == '[')
            {
                if (text[currIndex + 1] == '/')
                {
                    SpeechTextAttribute& attr = attributes.top ();
                    attributes.pop ();

                    attr.EndIndex = currIndex - newLinesCount;
                    m_Attributes.push_back (attr);

                    int close = text.find (']', currIndex);

                    text.erase (currIndex, close - currIndex + 1);
                }
                else
                {
                    int keyIndex = text.find ('=', currIndex + 1);
                    std::string key = text.substr (currIndex + 1, keyIndex - 1 - currIndex);
                    std::transform (key.begin (), key.end (), key.begin (), ::toupper);

                    int close = text.find (']', currIndex);
                    std::string value = text.substr (keyIndex + 1, close - 1 - keyIndex);
                    std::transform (value.begin (), value.end (), value.begin (), ::toupper);

                    SpeechTextAttribute attr;
                    attr.BeginIndex = currIndex - newLinesCount;

                    if (key == "COLOR")
                    {
                        if (value == "RED")
                        {
                            attr.Color = COLOR_RED;
                        }
                        else if (value == "YELLOW")
                        {
                            attr.Color = COLOR_YELLOW;
                        }
                        else if (value == "GREEN")
                        {
                            attr.Color = COLOR_GREEN;
                        }
                        else if (value == "LIGHTBLUE")
                        {
                            attr.Color = COLOR_LIGHTBLUE;
                        }
                    }

                    attributes.push (attr);

                    text.erase (currIndex, close - currIndex + 1);
                }
            }

            ++currIndex;
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool SpeechFrame::IsShouldBeHandled () { return m_ShouldBeHandled; }

    //--------------------------------------------------------------------------------------------------

    bool SpeechFrame::IsHandled () { return m_Handled; }

    //--------------------------------------------------------------------------------------------------
}
