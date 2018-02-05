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

    SpeechFrame::SpeechFrame (SpeechFrameManager* manager)
        : SpeechFrame (manager, "", Rect (), true, "")
    {
    }

    //--------------------------------------------------------------------------------------------------

    SpeechFrame::SpeechFrame (SpeechFrameManager* manager, const std::string& text, Rect rect, bool shouldBeHandled,
                              const std::string& regionName)
        : m_Manager (manager)
        , m_DrawRect (rect)
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
        , m_ShouldBeHandled (shouldBeHandled)
        , m_Handled (false)
        , m_ActorRegionName (regionName)
        , m_DelayCounter (0.0f)
        , m_IsDelayed (false)
        , m_ActualChoiceIndex (0)
    {
        m_FrameBitmap = load_nine_patch_bitmap (GetResourcePath (ResourceID::GFX_TEXT_FRAME).c_str ());

        if (m_ActorRegionName != "")
        {
            m_Atlas = m_Manager->GetSceneManager ()->GetAtlasManager ()->GetAtlas (
                GetBaseName (GetResourcePath (PACK_CHARACTERS_UI)));
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

        if (!IsTextFit () && m_KeyDelta > m_MaxKeyDelta)
        {
            ALLEGRO_KEYBOARD_STATE state;
            al_get_keyboard_state (&state);

            int maxLines = (m_DrawRect.GetSize ().Height - 2 * SPEECH_FRAME_TEXT_INSETS) / m_LineHeight;
            int lineCounter = GetLineCounter ();

            if (al_key_down (&state, ALLEGRO_KEY_UP) || al_key_down (&state, ALLEGRO_KEY_W))
            {
                //  We can only scroll, when there are no choices
                if (!m_Choices.empty () || m_Choices.size () >= maxLines)
                {
                    if (!m_StillUpdating)
                    {
                        --m_ActualChoiceIndex;

                        if (m_ActualChoiceIndex < 0)
                        {
                            m_ActualChoiceIndex = m_Choices.size () - 1;
                        }

                        if (ChoiceUpFunction)
                        {
                            ChoiceUpFunction ();
                        }
                    }
                }
                else
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
            }

            if (al_key_down (&state, ALLEGRO_KEY_DOWN) || al_key_down (&state, ALLEGRO_KEY_S))
            {
                //  We can only scroll, when there are no choices
                if (!m_Choices.empty () || m_Choices.size () >= maxLines)
                {
                    if (!m_StillUpdating)
                    {
                        ++m_ActualChoiceIndex;

                        if (m_ActualChoiceIndex > m_Choices.size () - 1)
                        {
                            m_ActualChoiceIndex = 0;
                        }

                        if (ChoiceDownFunction)
                        {
                            ChoiceDownFunction ();
                        }
                    }
                }
                else
                {
                    ++m_DisplayLine;

                    int diff = m_CurrentLine + 1 - maxLines;

                    if (lineCounter < diff)
                    {
                        if (ScrollDownFunction)
                        {
                            ScrollDownFunction ();
                        }
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
            if (m_IsDelayed)
            {
                m_DelayCounter -= deltaTime * 1000.0f;

                if (m_DelayCounter <= 0.0f)
                {
                    m_IsDelayed = false;
                    m_DelayCounter = 0.0f;
                }
            }

            m_CurrentDrawTime += deltaTime * 1000;

            if (m_CurrentDrawTime >= m_DrawSpeed)
            {
                m_CurrentDrawTime = 0.0f;

                if (!m_IsDelayed)
                {
                    ++m_CurrentIndex;
                }

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
            int maxLines = (m_DrawRect.GetSize ().Height - 2 * SPEECH_FRAME_TEXT_INSETS) / m_LineHeight;

            switch (event->keyboard.keycode)
            {
            case ALLEGRO_KEY_UP:
            {
                //  We can only scroll, when there are no choices
                if (!m_Choices.empty () || m_Choices.size () >= maxLines)
                {
                    if (!m_StillUpdating)
                    {
                        --m_ActualChoiceIndex;

                        if (m_ActualChoiceIndex < 0)
                        {
                            m_ActualChoiceIndex = m_Choices.size () - 1;
                        }

                        if (ChoiceUpFunction)
                        {
                            ChoiceUpFunction ();
                        }

                        m_KeyEventHandled = true;
                    }
                }
                else
                {
                    if (!IsTextFit ())
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
                    }
                }

                break;
            }

            case ALLEGRO_KEY_DOWN:
            {
                //  We can only scroll, when there are no choices
                if (!m_Choices.empty () || m_Choices.size () >= maxLines)
                {
                    if (!m_StillUpdating)
                    {
                        ++m_ActualChoiceIndex;

                        if (m_ActualChoiceIndex > m_Choices.size () - 1)
                        {
                            m_ActualChoiceIndex = 0;
                        }

                        if (ChoiceDownFunction)
                        {
                            ChoiceDownFunction ();
                        }

                        m_KeyEventHandled = true;
                    }
                }
                else
                {
                    if (!IsTextFit ())
                    {
                        ++m_DisplayLine;

                        int diff = m_CurrentLine + 1 - maxLines;

                        if (lineCounter < diff)
                        {
                            if (ScrollDownFunction)
                            {
                                ScrollDownFunction ();
                            }
                        }

                        m_KeyEventHandled = true;
                    }
                }

                break;
            }

            case ALLEGRO_KEY_ENTER:
            {
                if (m_ShouldBeHandled)
                {
                    if (!m_Choices.empty ())
                    {
                        std::function<void()>& handler = m_Choices[m_ActualChoiceIndex].Func;

                        if (handler)
                        {
                            handler ();
                        }
                    }

                    m_Handled = true;

                    if (HandledFunction)
                    {
                        HandledFunction ();
                    }
                }

                break;
            }
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    int SpeechFrame::GetLineCounter ()
    {
        int maxLines = (m_DrawRect.GetSize ().Height - 2 * SPEECH_FRAME_TEXT_INSETS) / m_LineHeight;
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

        Font& font = m_Manager->GetSceneManager ()->GetMainLoop ()->GetScreen ()->GetFont ();
        unsigned ascent = font.GetFontAscent (FONT_NAME_SPEECH_FRAME);
        unsigned descent = font.GetFontDescent (FONT_NAME_SPEECH_FRAME);

        m_LineHeight = ascent + descent;
        m_TextLines = BreakLine (m_Text, m_DrawRect.GetSize ().Width - 2 * SPEECH_FRAME_TEXT_INSETS);
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechFrame::AddChoice (const std::string& text, std::function<void()> func)
    {
        SpeechChoice choice;
        choice.Text = text;
        choice.Func = func;

        m_Choices.push_back (choice);

        SetText (m_Text + "\n" + text);
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechFrame::Render (float deltaTime)
    {
        if (!m_Visible)
        {
            return;
        }

        if (m_Atlas && m_ActorRegionName != "")
        {
            Point characterOffset = GetActorRegionOffset ();
            int edgeLength
                = std::min (m_DrawRect.GetSize ().Height - 2 * characterOffset.Y, SPEECH_FRAME_MAX_CHAR_EDGE_LENGTH);
            AtlasRegion region = m_Atlas->GetRegion (m_ActorRegionName);

            float ratio = std::min ((float)edgeLength / region.Bounds.Size.Width,
                                    (float)edgeLength / region.Bounds.Size.Height);

            m_Atlas->DrawRegion (m_ActorRegionName,
                                 m_DrawRect.GetPos ().X - ratio * region.Bounds.Size.Width * 0.5f - characterOffset.X,
                                 m_DrawRect.GetPos ().Y + m_DrawRect.GetSize ().Y * 0.5f, ratio, ratio, 0);
        }

        draw_nine_patch_bitmap (m_FrameBitmap, m_DrawRect.GetPos ().X, m_DrawRect.GetPos ().Y,
                                m_DrawRect.GetSize ().Width, m_DrawRect.GetSize ().Height);

        int xPoint;
        int yPoint;
        int align;
        Font& font = m_Manager->GetSceneManager ()->GetMainLoop ()->GetScreen ()->GetFont ();
        std::string text = m_Text.substr (0, m_CurrentIndex);

        int centerOffset = 0;

        if (m_DrawTextCenter)
        {
            Point textDimensions = font.GetTextDimensions (FONT_NAME_SPEECH_FRAME, text);

            xPoint = m_DrawRect.GetPos ().X + m_DrawRect.GetSize ().Width * 0.5;
            yPoint = m_DrawRect.GetPos ().Y + m_DrawRect.GetSize ().Height * 0.5 - textDimensions.Height * 0.5;
            align = ALLEGRO_ALIGN_CENTER;
            centerOffset = -textDimensions.Width * 0.5f;
        }
        else
        {
            xPoint = m_DrawRect.GetPos ().X + SPEECH_FRAME_TEXT_INSETS;
            yPoint = m_DrawRect.GetPos ().Y + SPEECH_FRAME_TEXT_INSETS;
            align = ALLEGRO_ALIGN_LEFT;
        }

        int maxLines = (m_DrawRect.GetSize ().Height - 2 * SPEECH_FRAME_TEXT_INSETS) / m_LineHeight;
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

        int x, y, w, h;
        al_get_clipping_rectangle (&x, &y, &w, &h);
        al_set_clipping_rectangle (m_DrawRect.GetPos ().X, m_DrawRect.GetPos ().Y, m_DrawRect.GetSize ().Width,
                                   m_DrawRect.GetSize ().Height - SPEECH_FRAME_LINE_OFFSET);

        int choiceStartY = -1;

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

            int y = yPoint + i * (m_LineHeight + SPEECH_FRAME_LINE_OFFSET);

            float advance = 0;

            if (m_DrawTextCenter)
            {
                advance = centerOffset;
            }

            if (lineCounter >= m_TextLines.size () - m_Choices.size ())
            {
                advance = SPEECH_FRAME_ADVANCE_CHOICE;

                if (choiceStartY < 0)
                {
                    choiceStartY = i;
                }
            }

            for (int j = 0; j < t.length (); ++j)
            {
                ALLEGRO_COLOR color = { 0.9f, 0.9f, 0.9f, 1.0f };

                for (int k = 0; k < m_Attributes.size (); ++k)
                {
                    SpeechTextAttribute& attr = m_Attributes[k];

                    if (currentCharIndex >= attr.BeginIndex && currentCharIndex <= attr.EndIndex)
                    {
                        if (attr.AttributesMask & ATTRIBUTE_COLOR)
                        {
                            color = attr.Color;
                        }

                        if (attr.AttributesMask & ATTRIBUTE_DELAY && !m_IsDelayed && attr.Delay > 0.0f)
                        {
                            m_IsDelayed = true;
                            m_DelayCounter = attr.Delay;
                        }

                        break;
                    }
                }

                ++currentCharIndex;

                std::string begin = t.substr (0, j);
                if (!(isspace (t[j]) && TrimString (begin) == ""))
                {
                    std::string charToDraw = std::string (1, t[j]);

                    font.DrawText (FONT_NAME_SPEECH_FRAME, color, xPoint + advance, y, charToDraw, align);

                    if (charToDraw == " ")
                    {
                        advance += SPEECH_FRAME_ADVANCE_SPACE;
                    }
                    else
                    {
                        advance += m_Manager->GetSceneManager ()
                                       ->GetMainLoop ()
                                       ->GetScreen ()
                                       ->GetFont ()
                                       .GetTextDimensions (FONT_NAME_SPEECH_FRAME, charToDraw)
                                       .Width;
                        advance += SPEECH_FRAME_ADVANCE_LETTERS;
                    }
                }
            }
        }

        al_set_clipping_rectangle (x, y, w, h);

        //  We can only scroll, when there are no choices
        if (m_Choices.empty () || m_Choices.size () > maxLines)
        {
            int xOffset = 20;
            int yOffset = 2;
            std::string regionName = m_DrawLightArrow ? "arrow_light" : "arrow_dark";

            //  Up arrow
            if (lineCounter > 0)
            {
                m_Manager->GetSceneManager ()
                    ->GetAtlasManager ()
                    ->GetAtlas (GetBaseName (GetResourcePath (PACK_MENU_UI)))
                    ->DrawRegion (regionName, m_DrawRect.GetPos ().X + m_DrawRect.GetSize ().Width - xOffset,
                                  m_DrawRect.GetPos ().Y + yOffset, 1.0f, 1.0f, DegressToRadians (180.0f));
            }

            //  Down arrow
            if (lineCounter < diff)
            {
                m_Manager->GetSceneManager ()
                    ->GetAtlasManager ()
                    ->GetAtlas (GetBaseName (GetResourcePath (PACK_MENU_UI)))
                    ->DrawRegion (regionName, m_DrawRect.GetPos ().X + m_DrawRect.GetSize ().Width - xOffset,
                                  m_DrawRect.GetPos ().Y + m_DrawRect.GetSize ().Height - yOffset, 1.0f, 1.0f, 0.0f);
            }
        }
        else
        {
            if (!m_StillUpdating)
            {
                float xPos = m_DrawRect.GetPos ().X + SPEECH_FRAME_ADVANCE_CHOICE - SPEECH_FRAME_TEXT_INSETS * 0.5f;
                float yPos = yPoint + (m_ActualChoiceIndex + choiceStartY) * (m_LineHeight + SPEECH_FRAME_LINE_OFFSET)
                    + m_LineHeight * 0.5f;

                m_Manager->GetSceneManager ()
                    ->GetAtlasManager ()
                    ->GetAtlas (GetBaseName (GetResourcePath (PACK_MENU_UI)))
                    ->DrawRegion ("arrow_light", xPos, yPos, 1.0f, 1.0f, DegressToRadians (-90.0f));
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    std::vector<std::string> SpeechFrame::BreakLine (const std::string& line, float maxWidth)
    {
        std::vector<std::string> ret;

        float width = m_Manager->GetSceneManager ()
                          ->GetMainLoop ()
                          ->GetScreen ()
                          ->GetFont ()
                          .GetTextDimensions (FONT_NAME_SPEECH_FRAME, line)
                          .Width;

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
                currentIndex = 0;
            }
            else
            {
                width = m_Manager->GetSceneManager ()
                            ->GetMainLoop ()
                            ->GetScreen ()
                            ->GetFont ()
                            .GetTextDimensions (FONT_NAME_SPEECH_FRAME, currentPart)
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
        m_DelayCounter = 0.0f;
        m_IsDelayed = false;
        m_ActualChoiceIndex = 0;
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

    bool SpeechFrame::IsTextFit ()
    {
        return m_TextLines.size () <= (m_DrawRect.GetSize ().Height - 2 * SPEECH_FRAME_TEXT_INSETS) / m_LineHeight;
    }

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

                    attr.EndIndex = currIndex - newLinesCount - 1;
                    m_Attributes.push_back (attr);

                    int close = text.find (']', currIndex);

                    text.erase (currIndex, close - currIndex + 1);
                    --currIndex;
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

                        attr.AttributesMask |= ATTRIBUTE_COLOR;
                    }
                    else if (key == "WAIT")
                    {
                        attr.Delay = atof (value.c_str ());
                        attr.BeginIndex -= 1;

                        attr.AttributesMask |= ATTRIBUTE_DELAY;
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

    Point SpeechFrame::GetActorRegionOffset () { return Point (20, 10); }

    //--------------------------------------------------------------------------------------------------
}
