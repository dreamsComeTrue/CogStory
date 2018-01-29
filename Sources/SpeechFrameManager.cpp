// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "SpeechFrameManager.h"
#include "AudioManager.h"
#include "AudioSample.h"
#include "MainLoop.h"
#include "Player.h"
#include "Scene.h"
#include "SceneManager.h"
#include "Screen.h"
#include "SpeechFrame.h"

namespace aga
{
    typedef std::map<std::string, SpeechFrame*>::iterator SpeechFrameIterator;

    //--------------------------------------------------------------------------------------------------

    SpeechFrameManager::SpeechFrameManager (SceneManager* sceneManager)
        : m_SceneManager (sceneManager)
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
            "SELECT_MENU", GetResourcePath (SOUND_MENU_SELECT));

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool SpeechFrameManager::Destroy ()
    {
        for (SpeechFrameIterator it = m_Frames.begin (); it != m_Frames.end (); ++it)
        {
            SAFE_DELETE (it->second);
        }

        return Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechFrameManager::ProcessEvent (ALLEGRO_EVENT* event, float deltaTime)
    {
        for (SpeechFrameIterator it = m_Frames.begin (); it != m_Frames.end (); ++it)
        {
            SpeechFrame* frame = it->second;

            if (frame->IsVisible ())
            {
                frame->ProcessEvent (event, deltaTime);

                if (frame->IsShouldBeHandled () && frame->IsHandled ())
                {
                    SAFE_DELETE (it->second);
                    m_Frames.erase (it);

                    m_SceneManager->GetPlayer ().SetPreventInput (false);
                }
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool SpeechFrameManager::Update (float deltaTime)
    {
        for (SpeechFrameIterator it = m_Frames.begin (); it != m_Frames.end (); ++it)
        {
            SpeechFrame* frame = it->second;

            if (frame->IsVisible ())
            {
                if (frame->IsShouldBeHandled ())
                {
                    m_SceneManager->GetPlayer ().SetPreventInput (true);
                }

                frame->Update (deltaTime);
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechFrameManager::Render (float deltaTime)
    {
        for (SpeechFrameIterator it = m_Frames.begin (); it != m_Frames.end (); ++it)
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
        m_Frames.clear ();

        if (m_SceneManager->GetPlayer ().IsPreventInput ())
        {
            m_SceneManager->GetPlayer ().SetPreventInput (false);
        }
    }

    //--------------------------------------------------------------------------------------------------

    SpeechFrame* SpeechFrameManager::AddSpeechFrame (const std::string& id, const std::string& text, Rect rect,
                                                     bool shouldBeHandled, const std::string& regionName)
    {
        if (m_Frames.find (id) == m_Frames.end ())
        {
            SpeechFrame* frame = new SpeechFrame (this, text, rect, shouldBeHandled, regionName);
            m_Frames.insert (std::make_pair (id, frame));

            frame->ScrollDownFunction = [&]() { m_SelectSample->Play (); };
            frame->ScrollUpFunction = [&]() { m_SelectSample->Play (); };
            frame->ChoiceUpFunction = [&]() { m_SelectSample->Play (); };
            frame->ChoiceDownFunction = [&]() { m_SelectSample->Play (); };
            frame->HandledFunction = [&]() { m_SelectSample->Play (); };
        }

        return m_Frames[id];
    }

    //--------------------------------------------------------------------------------------------------

    SpeechFrame* SpeechFrameManager::AddSpeechFrame (const std::string& id, const std::string& text, Point pos,
                                                     int maxLineCharsCount, int linesCount, bool shouldBeHandled,
                                                     const std::string& regionName)
    {
        Point size = GetTextRectSize (maxLineCharsCount, linesCount);

        Rect rect;
        rect.SetPos (pos);
        rect.SetSize (size.Width, size.Height);

        return AddSpeechFrame (id, text, rect, shouldBeHandled, regionName);
    }

    //--------------------------------------------------------------------------------------------------

    SpeechFrame* SpeechFrameManager::AddSpeechFrame (SpeechData* speechData, Point pos, int maxLineCharsCount,
                                                     int linesCount, bool shouldBeHandled,
                                                     const std::string& regionName)
    {
        return AddSpeechFrame (speechData->Name, speechData->Text[CURRENT_LANG], pos, maxLineCharsCount, linesCount,
                               shouldBeHandled, regionName);
    }

    //--------------------------------------------------------------------------------------------------

    SpeechFrame* SpeechFrameManager::AddSpeechFrame (const std::string& speechID, Point pos, int maxLineCharsCount,
                                                     int linesCount, bool shouldBeHandled)
    {
        SpeechData* speech = m_SceneManager->GetActiveScene ()->GetSpeech (speechID);
        SpeechFrame* frame = nullptr;

        if (speech)
        {
            frame = AddSpeechFrame (speech->Name, speech->Text[CURRENT_LANG], pos, maxLineCharsCount, linesCount,
                                    shouldBeHandled, speech->ActorRegionName);

            std::vector<SpeechOutcome>& outcomes = speech->Outcomes[CURRENT_LANG];

            for (SpeechOutcome& outcome : outcomes)
            {
                frame->AddChoice (outcome.Text, nullptr);
            }
        }

        return frame;
    }

    //--------------------------------------------------------------------------------------------------

    SpeechFrame* SpeechFrameManager::AddSpeechFrame (const std::string& speechID, SpeechFramePosition position,
                                                     int maxLineCharsCount, int linesCount, bool shouldBeHandled)
    {
        const int SCREEN_OFFSET = 10;
        Point screenSize = m_SceneManager->GetMainLoop ()->GetScreen ()->GetWindowSize ();
        Point size = GetTextRectSize (maxLineCharsCount, linesCount);
        Point pos;

        Point characterOffset = SpeechFrame::GetActorRegionOffset ();
        int edgeLength = size.Height - 2 * characterOffset.Y;

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
        }

        return AddSpeechFrame (speechID, pos, maxLineCharsCount, linesCount, shouldBeHandled);
    }

    //--------------------------------------------------------------------------------------------------

    SceneManager* SpeechFrameManager::GetSceneManager () { return m_SceneManager; }

    //--------------------------------------------------------------------------------------------------

    Point SpeechFrameManager::GetTextRectSize (int maxLineCharsCount, int linesCount)
    {
        Font& font = m_SceneManager->GetMainLoop ()->GetScreen ()->GetFont ();
        unsigned ascent = font.GetFontAscent (FONT_NAME_SPEECH_FRAME);
        unsigned descent = font.GetFontDescent (FONT_NAME_SPEECH_FRAME);

        Point letterDim
            = m_SceneManager->GetMainLoop ()->GetScreen ()->GetFont ().GetTextDimensions (FONT_NAME_SPEECH_FRAME, "X");

        float width = maxLineCharsCount * letterDim.Width + 2 * SPEECH_FRAME_TEXT_INSETS
            + (maxLineCharsCount - 1) * SPEECH_FRAME_ADVANCE_LETTERS;
        float height = linesCount * (ascent + descent + SPEECH_FRAME_LINE_OFFSET) + SPEECH_FRAME_LINE_OFFSET
            + SPEECH_FRAME_TEXT_INSETS;

        return { width, height };
    }

    //--------------------------------------------------------------------------------------------------
}
