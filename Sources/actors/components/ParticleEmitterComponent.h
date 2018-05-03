// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __PARTICLE_EMITTER_COMPONENT_H__
#define __PARTICLE_EMITTER_COMPONENT_H__

#include "Component.h"

namespace aga
{
    class ParticleEmitter;

    class ParticleEmitterComponent : public Component
    {
    public:
        static std::string TypeName;

    public:
        ParticleEmitterComponent (Actor* owner);

        virtual bool Destroy () override;

        virtual bool Update (float deltaTime) override;

        virtual bool Render (float deltaTime) override;

        void CreateEmitter (const std::string& atlasName, const std::string& atlasRegionName, 
                            unsigned maxParticles, unsigned emitLifeSpan);

        ParticleEmitter* GetEmitter () { return m_Emitter; }

        virtual std::string GetTypeName () override { return TypeName; }

    private:
        ParticleEmitter* m_Emitter;
    };
}

#endif //   __PARTICLE_EMITTER_COMPONENT_H__

