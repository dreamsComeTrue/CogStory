// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "SpeechFrameManager.h"
#include "AudioManager.h"
#include "AudioSample.h"
#include "MainLoop.h"
#include "Player.h"
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

    void SpeechFrameManager::Clear () { m_Frames.clear (); }

    //--------------------------------------------------------------------------------------------------

    SpeechFrame* SpeechFrameManager::AddSpeechFrame (const std::string& id,
                                                     const std::string& text,
                                                     Rect rect,
                                                     bool shouldBeHandled,
                                                     const std::string& regionName)
    {
        if (m_Frames.find (id) == m_Frames.end ())
        {
            SpeechFrame* frame = new SpeechFrame (this, text, rect, shouldBeHandled, regionName);
            m_Frames.insert (std::make_pair (id, frame));

            frame->ScrollDownFunction = [&]() { m_SelectSample->Play (); };
            frame->ScrollUpFunction = [&]() { m_SelectSample->Play (); };
            frame->HandledFunction = [&]() { m_SelectSample->Play (); };
        }

        return m_Frames[id];
    }

    //--------------------------------------------------------------------------------------------------

    SpeechFrame* SpeechFrameManager::AddSpeechFrame (const std::string& id,
                                                     const std::string& text,
                                                     Point pos,
                                                     int maxLineCharsCount,
                                                     int linesCount,
                                                     bool shouldBeHandled,
                                                     const std::string& regionName)
    {
        Font& font = m_SceneManager->GetMainLoop ()->GetScreen ()->GetFont ();
        unsigned ascent = font.GetFontAscent (FONT_NAME_MAIN_MEDIUM);
        unsigned descent = font.GetFontDescent (FONT_NAME_MAIN_MEDIUM);

        Point dims =
          m_SceneManager->GetMainLoop ()->GetScreen ()->GetFont ().GetTextDimensions (FONT_NAME_MAIN_MEDIUM, "X");

        Rect rect;
        rect.SetPos (pos);

        float width = maxLineCharsCount * dims.Width + 2 * SPEECH_FRAME_TEXT_INSETS +
                      (maxLineCharsCount + 1) * SPEECH_FRAME_ADVANCE_LETTERS;
        float height = linesCount * (ascent + descent + SPEECH_FRAME_LINE_OFFSET) + SPEECH_FRAME_LINE_OFFSET +
                       SPEECH_FRAME_TEXT_INSETS;

        rect.SetSize (width, height);

        return AddSpeechFrame (id, text, rect, shouldBeHandled, regionName);
    }

    //--------------------------------------------------------------------------------------------------

    SceneManager* SpeechFrameManager::GetSceneManager () { return m_SceneManager; }

    //--------------------------------------------------------------------------------------------------
}
