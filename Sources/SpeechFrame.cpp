// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "SpeechFrame.h"
#include "Atlas.h"
#include "AtlasManager.h"
#include "AudioSample.h"
#include "Font.h"
#include "MainLoop.h"
#include "Resources.h"
#include "Scene.h"
#include "SceneManager.h"
#include "Screen.h"
#include "Script.h"
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
        const std::string& actionName, const std::string& regionName)
        : m_Manager (manager)
        , m_DrawRect (rect)
        , m_Visible (true)
        , m_DrawTextCenter (false)
        , m_DrawSpeed (15)
        , m_ArrowDrawSpeed (300)
        , m_DrawLightArrow (true)
        , m_CurrentDrawTime (0)
        , m_CurrentIndex (0)
        , m_CurrentLine (0)
        , m_ChosenLineDelta (0)
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
        , m_AttrColorIndex (0)
        , m_AttrDelayIndex (0)
        , m_Action (actionName)
        , m_ScriptHandleFunction (nullptr)
        , m_OverallIndex (0)
    {
        Atlas* atlas = m_Manager->GetSceneManager ()->GetMainLoop ()->GetAtlasManager ().GetAtlas (
            GetBaseName (GetResourcePath (PACK_MENU_UI)));
        Rect atlasRect = atlas->GetRegion ("text_frame").Bounds;
        ALLEGRO_BITMAP* bmp = al_create_sub_bitmap (atlas->GetImage (), atlasRect.GetPos ().X, atlasRect.GetPos ().Y,
            atlasRect.GetSize ().Width, atlasRect.GetSize ().Height);

        m_FrameBitmap = create_nine_patch_bitmap (bmp, true);

        if (m_ActorRegionName != "")
        {
            m_Atlas = m_Manager->GetSceneManager ()->GetMainLoop ()->GetAtlasManager ().GetAtlas (
                GetBaseName (GetResourcePath (PACK_ACTORS_UI)));
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

    int SpeechFrame::GetMaxLinesCanFit ()
    {
        return (m_DrawRect.GetSize ().Height - 2 * SPEECH_FRAME_TEXT_INSETS) / m_LineHeight;
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechFrame::MoveChoiceUp ()
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

    //--------------------------------------------------------------------------------------------------

    void SpeechFrame::MoveChoiceDown ()
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

    //--------------------------------------------------------------------------------------------------

    void SpeechFrame::HandleKeyUp ()
    {
        if (!IsTextFitWithoutScroll ())
        {
            //  We can only scroll, when there are no choices
            if (!m_Choices.empty () && !m_StillUpdating)
            {
                MoveChoiceUp ();
            }
            else
            {
                if (GetCurrentDrawingLine () > 0)
                {
                    if (ScrollUpFunction)
                    {
                        ScrollUpFunction ();
                    }
                }

                --m_ChosenLineDelta;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechFrame::HandleKeyDown ()
    {
        if (!IsTextFitWithoutScroll ())
        {
            if (!m_Choices.empty () && !m_StillUpdating)
            {
                MoveChoiceDown ();
            }
            else
            {
                int diff = m_CurrentLine + 1 - GetMaxLinesCanFit ();

                if (GetCurrentDrawingLine () < diff)
                {
                    if (ScrollDownFunction)
                    {
                        ScrollDownFunction ();
                    }
                }

                ++m_ChosenLineDelta;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechFrame::UpdateScrollArrowsFlash (float deltaTime)
    {
        m_CurrentFlashTime += deltaTime * 1000;

        if (m_CurrentFlashTime >= m_ArrowDrawSpeed)
        {
            m_CurrentFlashTime = 0;
            m_DrawLightArrow = !m_DrawLightArrow;
        }
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechFrame::UpdateTextPosition (float deltaTime)
    {
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
                if (m_OverallIndex % 7 == 0)
                {
                    m_Manager->GetTypeSample ()->Play ();
                }

                ++m_OverallIndex;

                m_CurrentDrawTime = 0.0f;

                if (!m_IsDelayed)
                {
                    ++m_CurrentIndex;
                    ++m_AttrDelayIndex;
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
                    m_OverallIndex = 0;
                }
            }
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

        if ( //! IsTextFitWithoutScroll () &&
            m_KeyDelta > m_MaxKeyDelta)
        {
            ALLEGRO_KEYBOARD_STATE state;
            al_get_keyboard_state (&state);

            if (al_key_down (&state, ALLEGRO_KEY_UP) || al_key_down (&state, ALLEGRO_KEY_W))
            {
                HandleKeyUp ();
            }

            if (al_key_down (&state, ALLEGRO_KEY_DOWN) || al_key_down (&state, ALLEGRO_KEY_S))
            {
                HandleKeyDown ();
            }

            m_KeyDelta = 0.0f;
        }

        UpdateScrollArrowsFlash (deltaTime);
        UpdateTextPosition (deltaTime);

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechFrame::ProcessEvent (ALLEGRO_EVENT* event, float deltaTime)
    {
        if (event->type == ALLEGRO_EVENT_KEY_DOWN)
        {
            switch (event->keyboard.keycode)
            {
            case ALLEGRO_KEY_UP:
            case ALLEGRO_KEY_W:
            {
                HandleKeyUp ();
                m_KeyEventHandled = true;

                break;
            }

            case ALLEGRO_KEY_DOWN:
            case ALLEGRO_KEY_S:
            {
                HandleKeyDown ();
                m_KeyEventHandled = true;

                break;
            }

            case ALLEGRO_KEY_ENTER:
            case ALLEGRO_KEY_SPACE:
            {
                if (m_ShouldBeHandled)
                {
                    if (!m_Choices.empty ())
                    {
                        SpeechChoice& choice = m_Choices[m_ActualChoiceIndex];

                        std::function<void()>& handler = choice.Func;

                        if (handler)
                        {
                            handler ();
                        }

                        //  Check, if we are using plain OUTCOME reference,
                        //  or one with REGISTERED_CHOICE_PREFIX (usually '*' mark) as the prefix
                        //  which coresponds to function registered with 'RegisterChoiceFunction'
                        if (StartsWith (choice.Action, REGISTERED_CHOICE_PREFIX))
                        {
                            std::string actionName = choice.Action.substr (REGISTERED_CHOICE_PREFIX.length ());
                            std::map<std::string, asIScriptFunction*>& choiceFunctions
                                = m_Manager->GetSceneManager ()->GetActiveScene ()->GetChoiceFunctions ();
                            asIScriptFunction* func = choiceFunctions[actionName];

                            if (func)
                            {
                                asIScriptContext* ctx
                                    = m_Manager->GetSceneManager ()->GetMainLoop ()->GetScriptManager ().GetContext ();
                                ctx->Prepare (func);

                                if (ctx->Execute () == asEXECUTION_FINISHED)
                                {
                                    m_OutcomeAction = *(std::string*)ctx->GetReturnObject ();
                                }

                                ctx->Unprepare ();
                                ctx->GetEngine ()->ReturnContext (ctx);
                            }
                        }
                        else
                        {
                            m_OutcomeAction = choice.Action;
                        }
                    }
                    else
                    {
                        m_OutcomeAction = m_Action;
                    }

                    m_Handled = true;

                    if (m_ScriptHandleFunction)
                    {
                        asIScriptContext* ctx
                            = m_Manager->GetSceneManager ()->GetMainLoop ()->GetScriptManager ().GetContext ();
                        ctx->Prepare (m_ScriptHandleFunction);
                        ctx->Execute ();
                        ctx->Unprepare ();
                        ctx->GetEngine ()->ReturnContext (ctx);
                    }

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

    int SpeechFrame::GetCurrentDrawingLine ()
    {
        int maxLines = GetMaxLinesCanFit ();
        int currentDrawingLine = 0;

        //  How many lines are hidden
        int diff = (int)m_CurrentLine + 1 - maxLines;

        //  Offset current drawing start position by hidden lines
        if (m_CurrentLine + 1 > maxLines)
        {
            currentDrawingLine = diff;
        }

        //  Move line position pointer by keyboard
        currentDrawingLine += m_ChosenLineDelta;

        //  If we select line too far up
        if (currentDrawingLine < 0)
        {
            currentDrawingLine = 0;
            m_ChosenLineDelta = -diff;
        }

        //  If we select line too far down
        if (currentDrawingLine > diff && diff >= 0)
        {
            currentDrawingLine = diff;
            m_ChosenLineDelta = 0;
        }

        return currentDrawingLine;
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechFrame::SetText (const std::string& text)
    {
        m_TextLines.clear ();
        m_Attributes.clear ();

        m_OriginalText = text;
        m_Text = text;
        PreprocessText (m_Text);

        Font& font = m_Manager->GetSceneManager ()->GetMainLoop ()->GetScreen ()->GetFont ();
        unsigned ascent = font.GetFontAscent (FONT_NAME_SPEECH_FRAME);
        unsigned descent = font.GetFontDescent (FONT_NAME_SPEECH_FRAME);

        m_LineHeight = ascent + descent;
        m_TextLines = BreakLine (m_Text, m_DrawRect.GetSize ().Width - 2 * SPEECH_FRAME_TEXT_INSETS);
        m_AttrColorIndex = 0;
        m_AttrDelayIndex = 0;
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechFrame::AddChoice (const std::string& text, const std::string& action, std::function<void()> func)
    {
        SpeechChoice choice;
        choice.Text = text;
        choice.Action = action;
        choice.Func = func;

        m_Choices.push_back (choice);

        SetText (m_OriginalText + "\n" + text);
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechFrame::Render (float deltaTime)
    {
        if (!m_Visible)
        {
            return;
        }

        DrawActorSprite ();

        //  Draw background
        draw_nine_patch_bitmap (m_FrameBitmap, m_DrawRect.GetPos ().X, m_DrawRect.GetPos ().Y,
            m_DrawRect.GetSize ().Width, m_DrawRect.GetSize ().Height);

        int maxLines = GetMaxLinesCanFit ();
        int currentDrawingLine = GetCurrentDrawingLine ();
        int currentDrawingLineCopy = currentDrawingLine;

        int x, y, w, h;
        al_get_clipping_rectangle (&x, &y, &w, &h);
        al_set_clipping_rectangle (m_DrawRect.GetPos ().X, m_DrawRect.GetPos ().Y, m_DrawRect.GetSize ().Width,
            m_DrawRect.GetSize ().Height - SPEECH_FRAME_LINE_OFFSET);

        float yOffset = -1;

        for (int i = 0; currentDrawingLine < m_CurrentLine + 1; ++currentDrawingLine, ++i)
        {
            if ((currentDrawingLine > m_TextLines.size () - 1) || (i > maxLines))
            {
                break;
            }

            std::string line = m_TextLines[currentDrawingLine];

            if (currentDrawingLine >= m_CurrentLine)
            {
                line = line.substr (0, m_CurrentIndex);
            }

            Point drawPoint = GetNextDrawPoint (i);

            if (yOffset < 0 && currentDrawingLine >= m_TextLines.size () - m_Choices.size ())
            {
                yOffset = drawPoint.Y;
            }

            m_AttrColorIndex = 0;

            for (int j = 0; j < currentDrawingLine; ++j)
            {
                m_AttrColorIndex += m_TextLines[j].size ();
            }

            float advance = GetTextAdvance (currentDrawingLine);
            DrawTextLine (line, drawPoint, advance);
        }

        al_set_clipping_rectangle (x, y, w, h);

        //  We can only scroll, when there are no choices
        if (m_Choices.empty () || m_Choices.size () > maxLines)
        {
            DrawScrollArrows (currentDrawingLineCopy);
        }
        else
        {
            if (!m_StillUpdating)
            {
                DrawChoiceArrow (yOffset);
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechFrame::DrawActorSprite ()
    {
        if (m_Atlas && m_ActorRegionName != "")
        {
            Point characterOffset = GetActorRegionOffset ();
            int edgeLength
                = std::min (m_DrawRect.GetSize ().Height - 2 * characterOffset.Y, SPEECH_FRAME_MAX_CHAR_EDGE_LENGTH);
            AtlasRegion region = m_Atlas->GetRegion (m_ActorRegionName);

            float ratio = std::min (
                (float)edgeLength / region.Bounds.Size.Width, (float)edgeLength / region.Bounds.Size.Height);

            float xPos = m_DrawRect.GetPos ().X - ratio * region.Bounds.Size.Width * 0.5f - characterOffset.X;
            float yPos
                = m_DrawRect.GetPos ().Y + m_DrawRect.GetHalfSize ().Height - region.Bounds.GetHalfSize ().Height;

            m_Atlas->DrawRegion (m_ActorRegionName, xPos, yPos, ratio, ratio, 0);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechFrame::DrawTextLine (const std::string& line, Point drawPoint, int advance)
    {
        Font& font = m_Manager->GetSceneManager ()->GetMainLoop ()->GetScreen ()->GetFont ();

        for (int j = 0; j < line.length (); ++j)
        {
            ALLEGRO_COLOR color = {0.9f, 0.9f, 0.9f, 1.0f};

            for (int k = 0; k < m_Attributes.size (); ++k)
            {
                SpeechTextAttribute& attr = m_Attributes[k];

                if (m_AttrColorIndex + j >= attr.BeginIndex && m_AttrColorIndex + j <= attr.EndIndex)
                {
                    if (attr.AttributesMask & ATTRIBUTE_COLOR)
                    {
                        color = attr.Color;
                    }
                }

                if (m_AttrDelayIndex >= attr.BeginIndex && m_AttrDelayIndex <= attr.EndIndex)
                {
                    if (attr.AttributesMask & ATTRIBUTE_DELAY && !m_IsDelayed && attr.Delay > 0.0f)
                    {
                        m_IsDelayed = true;
                        m_DelayCounter = attr.Delay;
                    }
                }
            }

            std::string begin = line.substr (0, j);
            if (!(isspace (line[j]) && TrimString (begin) == ""))
            {
                std::string charToDraw = std::string (1, line[j]);

                font.DrawText (
                    FONT_NAME_SPEECH_FRAME, color, drawPoint.X + advance, drawPoint.Y, charToDraw, GetTextAlign ());

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

    //--------------------------------------------------------------------------------------------------

    void SpeechFrame::DrawChoiceArrow (float yOffset)
    {
        float xPos = m_DrawRect.GetPos ().X + SPEECH_FRAME_ADVANCE_CHOICE - SPEECH_FRAME_TEXT_INSETS * 0.5f;
        float yPos = yOffset + (m_ActualChoiceIndex) * (m_LineHeight + SPEECH_FRAME_LINE_OFFSET) + m_LineHeight * 0.5f;

        m_Manager->GetSceneManager ()
            ->GetMainLoop ()
            ->GetAtlasManager ()
            .GetAtlas (GetBaseName (GetResourcePath (PACK_MENU_UI)))
            ->DrawRegion ("arrow_light", xPos, yPos, 1.0f, 1.0f, DegressToRadians (-90.0f), false);
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechFrame::DrawScrollArrows (int currentDrawingLine)
    {
        int xOffset = 20;
        int yOffset = 2;
        std::string regionName = m_DrawLightArrow ? "arrow_light" : "arrow_dark";

        int maxLines = GetMaxLinesCanFit ();

        Atlas* atlas = m_Manager->GetSceneManager ()->GetMainLoop ()->GetAtlasManager ().GetAtlas (
            GetBaseName (GetResourcePath (PACK_MENU_UI)));

        //  Up arrow
        if (currentDrawingLine > 0)
        {
            atlas->DrawRegion (regionName, m_DrawRect.GetPos ().X + m_DrawRect.GetSize ().Width - xOffset,
                m_DrawRect.GetPos ().Y + yOffset, 1.0f, 1.0f, DegressToRadians (180.0f), false);
        }

        //  How many lines are hidden
        int diff = (int)m_CurrentLine + 1 - maxLines;

        //  Down arrow
        if (currentDrawingLine < diff)
        {
            atlas->DrawRegion (regionName, m_DrawRect.GetPos ().X + m_DrawRect.GetSize ().Width - xOffset,
                m_DrawRect.GetPos ().Y + m_DrawRect.GetSize ().Height - yOffset, 1.0f, 1.0f, 0.0f, false);
        }
    }

    //--------------------------------------------------------------------------------------------------

    Point SpeechFrame::GetNextDrawPoint (int lineIndex)
    {
        float xPoint;
        float yPoint;
        Font& font = m_Manager->GetSceneManager ()->GetMainLoop ()->GetScreen ()->GetFont ();
        std::string text = m_Text.substr (0, m_CurrentIndex);

        if (m_DrawTextCenter)
        {
            Point textDimensions = font.GetTextDimensions (FONT_NAME_SPEECH_FRAME, text);

            xPoint = m_DrawRect.GetPos ().X + m_DrawRect.GetSize ().Width * 0.5;
            yPoint = m_DrawRect.GetPos ().Y + m_DrawRect.GetSize ().Height * 0.5 - textDimensions.Height * 0.5;
        }
        else
        {
            xPoint = m_DrawRect.GetPos ().X + SPEECH_FRAME_TEXT_INSETS;
            yPoint = m_DrawRect.GetPos ().Y + SPEECH_FRAME_TEXT_INSETS;
        }

        yPoint = yPoint + lineIndex * (m_LineHeight + SPEECH_FRAME_LINE_OFFSET);

        return {xPoint, yPoint};
    }

    //--------------------------------------------------------------------------------------------------

    int SpeechFrame::GetTextAlign () { return m_DrawTextCenter ? ALLEGRO_ALIGN_CENTER : ALLEGRO_ALIGN_LEFT; }

    //--------------------------------------------------------------------------------------------------

    float SpeechFrame::GetTextAdvance (int lineCounter)
    {
        float advance = 0;

        if (m_DrawTextCenter)
        {
            Font& font = m_Manager->GetSceneManager ()->GetMainLoop ()->GetScreen ()->GetFont ();
            std::string text = m_Text.substr (0, m_CurrentIndex);
            Point textDimensions = font.GetTextDimensions (FONT_NAME_SPEECH_FRAME, text);
            advance = -textDimensions.Width * 0.5f;
        }

        if (lineCounter >= m_TextLines.size () - m_Choices.size ())
        {
            advance = SPEECH_FRAME_ADVANCE_CHOICE;
        }

        return advance;
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

                    if (currentIndex != std::string::npos)
                    {
                        std::string str = workLine.substr (0, currentIndex);
                        ret.push_back (str);
                        workLine = workLine.substr (currentIndex);
                        currentIndex = 0;
                    }
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
        m_AttrColorIndex = 0;
        m_AttrDelayIndex = 0;
        m_CurrentLine = 0;
        m_CurrentDrawTime = 0;
        m_ChosenLineDelta = 0;
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

    bool SpeechFrame::IsTextFitWithoutScroll () { return m_TextLines.size () <= GetMaxLinesCanFit (); }

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

                    size_t close = text.find (']', currIndex);

                    text.erase (currIndex, close - currIndex + 1);
                    --currIndex;
                }
                else
                {
                    size_t keyIndex = text.find ('=', currIndex + 1);
                    std::string key = text.substr (currIndex + 1, keyIndex - 1 - currIndex);
                    std::transform (key.begin (), key.end (), key.begin (), ::toupper);

                    size_t close = text.find (']', currIndex);
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

                        if (attr.BeginIndex < 0)
                        {
                            attr.BeginIndex = 0;
                        }

                        attr.AttributesMask |= ATTRIBUTE_DELAY;
                    }

                    attributes.push (attr);

                    text.erase (currIndex, close - currIndex + 1);
                    currIndex -= 1;

                    if (currIndex < 0)
                    {
                        currIndex = 0;
                    }
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
