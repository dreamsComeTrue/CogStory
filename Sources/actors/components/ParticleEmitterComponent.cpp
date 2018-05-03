// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "ParticleEmitterComponent.h"
#include "ParticleEmitter.h"
#include "SceneManager.h"
#include "MainLoop.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    std::string ParticleEmitterComponent::TypeName = "ParticleEmitterComponent";

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    ParticleEmitterComponent::ParticleEmitterComponent (Actor* owner) :
        Component (owner),
        m_Emitter (nullptr)
    {
    }

    //--------------------------------------------------------------------------------------------------

    bool ParticleEmitterComponent::Destroy ()
    {
        SAFE_DELETE (m_Emitter);
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

    void ParticleEmitterComponent::CreateEmitter (const std::string& atlasName, 
            const std::string& atlasRegionName, unsigned maxParticles, unsigned emitLifeSpan)
    {
        m_Emitter = new ParticleEmitter (&m_Actor->GetSceneManager ()->GetMainLoop ()-> GetAtlasManager (), 
                                         atlasName, atlasRegionName, maxParticles, emitLifeSpan);
    }

    //--------------------------------------------------------------------------------------------------
}

