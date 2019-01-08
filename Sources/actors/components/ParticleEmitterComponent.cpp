// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "ParticleEmitterComponent.h"
#include "MainLoop.h"
#include "ParticleEmitter.h"

namespace aga
{
	//--------------------------------------------------------------------------------------------------

	std::string ParticleEmitterComponent::TypeName = "ParticleEmitterComponent";

	//--------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------

	ParticleEmitterComponent::ParticleEmitterComponent (Actor* owner)
		: Component (owner)
		, m_Emitter (nullptr)
	{
	}

	//--------------------------------------------------------------------------------------------------

	ParticleEmitterComponent::ParticleEmitterComponent (const ParticleEmitterComponent& rhs)
		: Component (rhs.m_Actor)
	{
		this->m_Emitter = rhs.m_Emitter;
	}

	//--------------------------------------------------------------------------------------------------

	ParticleEmitterComponent* ParticleEmitterComponent::Clone () const { return new ParticleEmitterComponent (*this); }

	//--------------------------------------------------------------------------------------------------

	bool ParticleEmitterComponent::Destroy ()
	{
		SAFE_DELETE (m_Emitter);

		return Component::Destroy ();
	}

	//--------------------------------------------------------------------------------------------------

	bool ParticleEmitterComponent::Update (float deltaTime)
	{
		if (!m_IsEnabled)
		{
			return true;
		}

		if (m_Emitter)
		{
			m_Emitter->Update (deltaTime);
		}

		return true;
	}

	//--------------------------------------------------------------------------------------------------

	bool ParticleEmitterComponent::Render (float deltaTime)
	{
		if (!m_IsEnabled)
		{
			return true;
		}

		if (m_Emitter)
		{
			m_Emitter->Render (deltaTime);
		}

		return true;
	}

	//--------------------------------------------------------------------------------------------------

	void ParticleEmitterComponent::CreateEmitter (
		const std::string& atlasName, const std::string& atlasRegionName, unsigned maxParticles, unsigned emitLifeSpan)
	{
		m_Emitter = new ParticleEmitter (&m_Actor->GetSceneManager ()->GetMainLoop ()->GetAtlasManager (), atlasName,
			atlasRegionName, maxParticles, emitLifeSpan);
		m_Emitter->SetPosition (m_Actor->Bounds.GetPos () + m_Actor->Bounds.GetHalfSize ());
	}

	//--------------------------------------------------------------------------------------------------
}
