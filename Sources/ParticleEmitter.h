// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __PARTICLE_EMITTER_H__
#define __PARTICLE_EMITTER_H__

#include "Common.h"

namespace aga
{
    struct Particle
    {
        Point Position;
        Point Velocity;
        ALLEGRO_COLOR Color;
        float Rotation = 0.0f;
        float MaxLife = 1.0f;
        float CurrentLife = 1.0f;
    };

    class ParticleEmitter : public Lifecycle
    {
    public:
        ParticleEmitter (unsigned maxParticles, unsigned emitLifeSpan, const std::string& imagePath);
        virtual ~ParticleEmitter ();
        bool Initialize ();
        bool Destroy ();

        bool Update (float deltaTime);
        void Render (float deltaTime);

        void SetCanEmit (bool canEmit);
        bool IsCanEmit ();
        void SetPosition (Point position);
        void SetPosition (float x, float y);
        void SetParticleLifeVariance (float minLife, float maxLife);
        void SetVelocityVariance (Point minVariance, Point maxVariance);
        void SetColorTransition (ALLEGRO_COLOR beginColor, ALLEGRO_COLOR endColor);

    private:
        void EmitParticle ();

    private:
        std::vector<Particle> m_Particles;
        bool m_CanEmit;
        Point m_Position;
        unsigned m_MaxParticles;
        float m_EmitLifeSpan;
        float m_ParticleMinLife;
        float m_ParticleMaxLife;
        Point m_VelocityMinVariance;
        Point m_VelocityMaxVariance;
        ALLEGRO_COLOR m_BeginColor;
        ALLEGRO_COLOR m_EndColor;

        std::string m_ImagePath;
        ALLEGRO_BITMAP* m_Image;
    };
}

#endif //   __PARTICLE_EMITTER_H__
