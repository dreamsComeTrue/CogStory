// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "SpeechFrameManager.h"
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

    bool SpeechFrameManager::Initialize () { return Lifecycle::Initialize (); }

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
            it->second->ProcessEvent (event, deltaTime);
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool SpeechFrameManager::Update (float deltaTime)
    {
        for (SpeechFrameIterator it = m_Frames.begin (); it != m_Frames.end (); ++it)
        {
            it->second->Update (deltaTime);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechFrameManager::Render (float deltaTime)
    {
        for (SpeechFrameIterator it = m_Frames.begin (); it != m_Frames.end (); ++it)
        {
            it->second->Render (deltaTime);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechFrameManager::Clear () { m_Frames.clear (); }

    //--------------------------------------------------------------------------------------------------

    SpeechFrame* SpeechFrameManager::AddSpeechFrame (const std::string& id, const std::string& text, Rect rect)
    {
        if (m_Frames.find (id) == m_Frames.end ())
        {
            SpeechFrame* frame = new SpeechFrame (this, text, rect);
            m_Frames.insert (std::make_pair (id, frame));
        }

        return m_Frames[id];
    }

    //--------------------------------------------------------------------------------------------------

    SceneManager* SpeechFrameManager::GetSceneManager () { return m_SceneManager; }

    //--------------------------------------------------------------------------------------------------
}
