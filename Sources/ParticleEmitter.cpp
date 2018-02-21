// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "ParticleEmitter.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    ParticleEmitter::ParticleEmitter (unsigned maxParticles, unsigned emitLifeSpan, const std::string& imagePath)
        : m_CanEmit (true)
        , m_MaxParticles (maxParticles)
        , m_EmitLifeSpan (emitLifeSpan)
        , m_ImagePath (imagePath)
        , m_ParticleMinLife (1.0f)
        , m_ParticleMaxLife (1.0f)
        , m_VelocityMinVariance (-1.0f, -1.0f)
        , m_VelocityMaxVariance (1.0f, 1.0f)
        , m_BeginColor (COLOR_WHITE)
        , m_EndColor (COLOR_WHITE)
    {
    }

    //--------------------------------------------------------------------------------------------------

    ParticleEmitter::~ParticleEmitter ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool ParticleEmitter::Initialize ()
    {
        Lifecycle::Initialize ();

        m_Image = al_load_bitmap (m_ImagePath.c_str ());

        if (m_CanEmit)
        {
            for (unsigned i = 0; i < m_MaxParticles; ++i)
            {
                EmitParticle ();
            }
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool ParticleEmitter::Destroy ()
    {
        if (m_Image != nullptr)
        {
            al_destroy_bitmap (m_Image);
            m_Image = nullptr;
        }

        return Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    bool ParticleEmitter::Update (float deltaTime)
    {
        for (int i = 0; i < m_Particles.size (); ++i)
        {
            Particle& particle = m_Particles[i];

            particle.CurrentLife -= deltaTime;

            if (particle.CurrentLife <= 0.0f)
            {
                m_Particles.erase (m_Particles.begin () + i);
            }
            else
            {
                particle.Position.X += particle.Velocity.X;
                particle.Position.Y -= particle.Velocity.Y;
                particle.Color = Lerp (m_BeginColor, m_EndColor, 1.0f - (particle.CurrentLife / particle.MaxLife));
            }
        }

        if (m_CanEmit && m_Particles.size () < m_MaxParticles)
        {
            EmitParticle ();
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void ParticleEmitter::Render (float deltaTime)
    {
        int blendOp, blendSrc, blendDst;
        al_get_blender (&blendOp, &blendSrc, &blendDst);
        al_set_blender (ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

        for (int i = 0; i < m_Particles.size (); ++i)
        {
            Particle& particle = m_Particles[i];
            float sourceWidth = al_get_bitmap_width (m_Image);
            float sourceHeight = al_get_bitmap_height (m_Image);

            al_draw_tinted_scaled_rotated_bitmap (m_Image, particle.Color, sourceWidth * 0.5, sourceHeight * 0.5,
                                                  particle.Position.X, particle.Position.Y, 1, 1, particle.Rotation, 0);
        }

        al_set_blender (blendOp, blendSrc, blendDst);
    }

    //--------------------------------------------------------------------------------------------------

    void ParticleEmitter::Reset () { m_Particles.clear (); }

    //--------------------------------------------------------------------------------------------------

    void ParticleEmitter::SetCanEmit (bool canEmit) { m_CanEmit = canEmit; }

    //--------------------------------------------------------------------------------------------------

    bool ParticleEmitter::IsCanEmit () { return m_CanEmit; }

    //--------------------------------------------------------------------------------------------------

    void ParticleEmitter::SetPosition (Point position) { m_Position = position; }

    //--------------------------------------------------------------------------------------------------

    void ParticleEmitter::SetPosition (float x, float y)
    {
        m_Position.X = x;
        m_Position.Y = y;
    }

    //--------------------------------------------------------------------------------------------------

    void ParticleEmitter::SetParticleLifeVariance (float minLife, float maxLife)
    {
        m_ParticleMinLife = minLife;
        m_ParticleMaxLife = maxLife;
    }

    //--------------------------------------------------------------------------------------------------

    void ParticleEmitter::SetVelocityVariance (Point minVariance, Point maxVariance)
    {
        m_VelocityMinVariance = minVariance;
        m_VelocityMaxVariance = maxVariance;
    }

    //--------------------------------------------------------------------------------------------------

    void ParticleEmitter::SetColorTransition (ALLEGRO_COLOR beginColor, ALLEGRO_COLOR endColor)
    {
        m_BeginColor = beginColor;
        m_EndColor = endColor;
    }

    //--------------------------------------------------------------------------------------------------

    void ParticleEmitter::EmitParticle ()
    {
        if (m_CanEmit && m_Particles.size () < m_MaxParticles)
        {
            Particle particle;
            particle.MaxLife = RandInRange (m_ParticleMinLife, m_ParticleMaxLife);
            particle.CurrentLife = particle.MaxLife;
            particle.Position = m_Position;
            particle.Velocity.X = RandInRange (m_VelocityMinVariance.X, m_VelocityMaxVariance.X);
            particle.Velocity.Y = RandInRange (m_VelocityMinVariance.Y, m_VelocityMaxVariance.Y);
            particle.Color = m_BeginColor;

            m_Particles.push_back (particle);
        }
    }

    //--------------------------------------------------------------------------------------------------
}
