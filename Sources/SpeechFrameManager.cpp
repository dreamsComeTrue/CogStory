// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "SpeechFrameManager.h"
#include "AudioSample.h"
#include "MainLoop.h"
#include "Player.h"
#include "Resources.h"
#include "Scene.h"
#include "Screen.h"
#include "SpeechFrame.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    typedef std::map<std::string, SpeechFrame*>::iterator SpeechFrameIterator;

    //--------------------------------------------------------------------------------------------------

    SpeechFrameManager::SpeechFrameManager (SceneManager* sceneManager)
        : m_SceneManager (sceneManager)
        , m_SpeechesFinishedScriptFunc (nullptr)
        , m_SpeechesFinishedFunc (nullptr)
        , m_SelectSample (nullptr)
        , m_TypeSample (nullptr)
    {
    }

    //--------------------------------------------------------------------------------------------------

    SpeechFrameManager::~SpeechFrameManager ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool SpeechFrameManager::Initialize ()
    {
        Lifecycle::Initialize ();

        m_SelectSample = m_SceneManager->GetMainLoop ()->GetAudioManager ().LoadSampleFromFile (
            "SPEECH_SELECT", GetResource (SOUND_SPEECH_SELECT).Dir + GetResource (SOUND_SPEECH_SELECT).Name);
        m_TypeSample = m_SceneManager->GetMainLoop ()->GetAudioManager ().LoadSampleFromFile (
            "SPEECH_TYPE", GetResource (SOUND_SPEECH_TYPE).Dir + GetResource (SOUND_SPEECH_TYPE).Name);

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool SpeechFrameManager::Destroy ()
    {
        for (SpeechFrameIterator it = m_Speeches.begin (); it != m_Speeches.end ();)
        {
            SAFE_DELETE (it->second);
            m_Speeches.erase (it++);
        }

        return Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    bool SpeechFrameManager::ProcessEvent (ALLEGRO_EVENT* event, float deltaTime)
    {
        std::vector<SpeechFrame*> nextSpeeches;

        for (SpeechFrameIterator it = m_Speeches.begin (); it != m_Speeches.end ();)
        {
            SpeechFrame* frame = it->second;

            if (frame->IsVisible ())
            {
                frame->ProcessEvent (event, deltaTime);

                if (frame->IsShouldBeHandled () && frame->IsHandled ()
                    && std::find (nextSpeeches.begin (), nextSpeeches.end (), frame) == nextSpeeches.end ())
                {
                    std::string outcomeAction = frame->GetOutcomeAction ();

                    SAFE_DELETE (it->second);
                    m_Speeches.erase (it++);

                    m_SceneManager->GetPlayer ()->SetPreventInput (false);

                    if (outcomeAction != "" && outcomeAction != "[CLOSE]")
                    {
                        SpeechFrame* frame = AddSpeechFrame (outcomeAction);

                        nextSpeeches.push_back (frame);
                        m_Speeches.insert (std::make_pair (outcomeAction, frame));
                    }
                    else
                    {
                        m_SelectSample->Play ();
                    }

                    continue;
                }
            }

            ++it;
        }

        for (SpeechFrame* nextSpeech : nextSpeeches)
        {
            nextSpeech->Show ();
        }

        if (m_SpeechesFinishedFunc && m_Speeches.empty ())
        {
            m_SpeechesFinishedFunc ();
        }

        if (m_SpeechesFinishedScriptFunc && m_Speeches.empty ())
        {
            asIScriptContext* ctx = m_SceneManager->GetMainLoop ()->GetScriptManager ().GetContext ();
            ctx->Prepare (m_SpeechesFinishedScriptFunc);
            ctx->Execute ();
            ctx->Unprepare ();
            ctx->GetEngine ()->ReturnContext (ctx);
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    bool SpeechFrameManager::Update (float deltaTime)
    {
        for (SpeechFrameIterator it = m_Speeches.begin (); it != m_Speeches.end (); ++it)
        {
            SpeechFrame* frame = it->second;

            if (frame->IsVisible ())
            {
                if (frame->IsShouldBeHandled ())
                {
                    m_SceneManager->GetPlayer ()->SetPreventInput (true);
                }

                frame->Update (deltaTime);
            }
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechFrameManager::Render (float deltaTime)
    {
        for (SpeechFrameIterator it = m_Speeches.begin (); it != m_Speeches.end (); ++it)
        {
            SpeechFrame* frame = it->second;

            if (frame->IsVisible ())
            {
                frame->Render (deltaTime);
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechFrameManager::Clear ()
    {
        m_Speeches.clear ();
    }

    //--------------------------------------------------------------------------------------------------

    SpeechFrame* SpeechFrameManager::AddSpeechFrame (const std::string& id, const std::string& text, Rect rect,
        bool shouldBeHandled, const std::string& actionName, const std::string& regionName)
    {
        if (m_Speeches.find (id) == m_Speeches.end ())
        {
            SpeechFrame* frame = new SpeechFrame (this, text, rect, shouldBeHandled, actionName, regionName);
            m_Speeches.insert (std::make_pair (id, frame));

            frame->ScrollDownFunction = [&]() { m_SelectSample->Play (); };
            frame->ScrollUpFunction = [&]() { m_SelectSample->Play (); };
            frame->ChoiceUpFunction = [&]() { m_SelectSample->Play (); };
            frame->ChoiceDownFunction = [&]() { m_SelectSample->Play (); };
        }

        return m_Speeches[id];
    }

    //--------------------------------------------------------------------------------------------------

    SpeechFrame* SpeechFrameManager::AddSpeechFrame (const std::string& id, const std::string& text, Point pos,
        int maxLineCharsCount, int linesCount, bool shouldBeHandled, const std::string& actionName,
        const std::string& regionName)
    {
        Point size = GetTextRectSize (maxLineCharsCount, linesCount);

        Rect rect;
        rect.SetPos (pos);
        rect.SetSize (size.Width, size.Height);

        return AddSpeechFrame (id, text, rect, shouldBeHandled, actionName, regionName);
    }

    //--------------------------------------------------------------------------------------------------

    SpeechFrame* SpeechFrameManager::AddSpeechFrame (SpeechData* speechData, bool shouldBeHandled)
    {
        Point pos = GetFramePos (speechData->RelativeFramePosition, speechData->AbsoluteFramePosition,
            speechData->MaxCharsInLine, speechData->MaxLines, speechData->ActorRegionName != "");

        return AddSpeechFrame (speechData->Name, speechData->Text[CURRENT_LANG], pos, speechData->MaxCharsInLine,
            speechData->MaxLines, shouldBeHandled, speechData->ActorRegionName);
    }

    //--------------------------------------------------------------------------------------------------

    SpeechFrame* SpeechFrameManager::AddSpeechFrame (const std::string& speechID, Point pos, bool shouldBeHandled)
    {
        SpeechData* instance = m_SceneManager->GetActiveScene ()->GetSpeech (speechID);
        SpeechFrame* frame = nullptr;

        if (instance)
        {
            frame = AddSpeechFrame (instance->Name, instance->Text[CURRENT_LANG], pos, instance->MaxCharsInLine,
                instance->MaxLines, shouldBeHandled, instance->Action, instance->ActorRegionName);

            std::vector<SpeechOutcome>& outcomes = instance->Outcomes[CURRENT_LANG];

            for (SpeechOutcome& outcome : outcomes)
            {
                frame->AddChoice (outcome.Text, outcome.Action, nullptr);
            }
        }

        return frame;
    }

    //--------------------------------------------------------------------------------------------------

    SpeechFrame* SpeechFrameManager::AddSpeechFrame (const std::string& speechID, bool shouldBeHandled)
    {
        SpeechData* instance = m_SceneManager->GetActiveScene ()->GetSpeech (speechID);

        if (instance)
        {
            Point pos = GetFramePos (instance->RelativeFramePosition, instance->AbsoluteFramePosition,
                instance->MaxCharsInLine, instance->MaxLines, instance->ActorRegionName != "");

            return AddSpeechFrame (speechID, pos, shouldBeHandled);
        }

        return nullptr;
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechFrameManager::RemoveSpeechFrame (const std::string& speechID)
    {
        for (SpeechFrameIterator it = m_Speeches.begin (); it != m_Speeches.end (); ++it)
        {
            if (it->first == speechID)
            {
                m_Speeches.erase (it);
                SAFE_DELETE (it->second);

                break;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    SceneManager* SpeechFrameManager::GetSceneManager () { return m_SceneManager; }

    //--------------------------------------------------------------------------------------------------

    Point SpeechFrameManager::GetFramePos (
        ScreenRelativePosition position, Point absolutePos, int maxCharsInLine, int maxLines, bool showActor)
    {
        const int SCREEN_OFFSET = 10;
        Point screenSize = m_SceneManager->GetMainLoop ()->GetScreen ()->GetBackBufferSize ();
        Point size = GetTextRectSize (maxCharsInLine, maxLines);
        Point pos;

        int edgeLength = 0;

        if (showActor)
        {
            Point characterOffset = SpeechFrame::GetActorRegionOffset ();
            edgeLength
                = static_cast<int> (std::min (size.Height - 2 * characterOffset.Y, SPEECH_FRAME_MAX_CHAR_EDGE_LENGTH));
        }

        switch (position)
        {
        case TopLeft:
        {
            pos.X = SCREEN_OFFSET + edgeLength;
            pos.Y = SCREEN_OFFSET;
            break;
        }

        case TopCenter:
        {
            pos.X = screenSize.Width * 0.5f - size.Width * 0.5f;
            pos.Y = SCREEN_OFFSET;
            break;
        }

        case TopRight:
        {
            pos.X = screenSize.Width - size.Width - SCREEN_OFFSET;
            pos.Y = SCREEN_OFFSET;
            break;
        }

        case BottomLeft:
        {
            pos.X = SCREEN_OFFSET + edgeLength;
            pos.Y = screenSize.Height - size.Height - SCREEN_OFFSET;
            break;
        }

        case BottomCenter:
        {
            pos.X = screenSize.Width * 0.5f - size.Width * 0.5f;
            pos.Y = screenSize.Height - size.Height - SCREEN_OFFSET;
            break;
        }

        case BottomRight:
        {
            pos.X = screenSize.Width - size.Width - SCREEN_OFFSET;
            pos.Y = screenSize.Height - size.Height - SCREEN_OFFSET;
            break;
        }

        case Center:
        {
            pos.X = screenSize.Width * 0.5f - size.Width * 0.5f;
            pos.Y = screenSize.Height * 0.5f - size.Height * 0.5f;
            break;
        }

        case Absoulte:
        {
            pos = absolutePos;
            break;
        }
        }

        return pos;
    }

    //--------------------------------------------------------------------------------------------------

    Point SpeechFrameManager::GetTextRectSize (int maxLineCharsCount, int linesCount)
    {
        Font& font = m_SceneManager->GetMainLoop ()->GetScreen ()->GetFont ();
        int ascent = font.GetFontAscent (FONT_NAME_SPEECH_FRAME);
        int descent = font.GetFontDescent (FONT_NAME_SPEECH_FRAME);

        float width = maxLineCharsCount * 25 + 2 * SPEECH_FRAME_TEXT_INSETS;
        float height = linesCount * (ascent + descent) - descent + 2 * SPEECH_FRAME_TEXT_INSETS;
        height += (linesCount - 1) * SPEECH_FRAME_LINE_OFFSET;

        return {width, height};
    }

    //--------------------------------------------------------------------------------------------------
}
