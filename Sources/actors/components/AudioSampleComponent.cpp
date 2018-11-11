// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "AudioSampleComponent.h"
#include "AudioSample.h"
#include "MainLoop.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    std::string AudioSampleComponent::TypeName = "AudioSampleComponent";

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    AudioSampleComponent::AudioSampleComponent (Actor* owner)
        : Component (owner)
        , m_Sample (nullptr)
    {
    }

    //--------------------------------------------------------------------------------------------------

    bool AudioSampleComponent::Update (float deltaTime)
    {
        m_Sample->Update (deltaTime);

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool AudioSampleComponent::Render (float) { return true; }

    //--------------------------------------------------------------------------------------------------

    void AudioSampleComponent::LoadSampleFromFile (const std::string& sampleName, const std::string& path)
    {
        m_Sample
            = m_Actor->GetSceneManager ()->GetMainLoop ()->GetAudioManager ().LoadSampleFromFile (sampleName, path);
    }

    //--------------------------------------------------------------------------------------------------
}
