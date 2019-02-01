// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

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

	AudioSampleComponent::AudioSampleComponent (const AudioSampleComponent& rhs)
		: Component (rhs.m_Actor)
	{
        this->m_Sample = rhs.m_Sample;
	}

	//--------------------------------------------------------------------------------------------------

	AudioSampleComponent* AudioSampleComponent::Clone () const { return new AudioSampleComponent (*this); }

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
