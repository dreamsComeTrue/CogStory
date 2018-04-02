// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Animable.h"
#include "Transformable.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    Animable::Animable ()
        : m_Image (nullptr)
    {
    }

    //--------------------------------------------------------------------------------------------------

    Animable::~Animable () { Destroy (); }

    //--------------------------------------------------------------------------------------------------

    bool Animable::Initialize (const std::string& filePath)
    {
        m_ImagePath = filePath;
        m_Image = al_load_bitmap (filePath.c_str ());
        m_ImageWidth = al_get_bitmap_width (m_Image);
        m_ImageHeight = al_get_bitmap_height (m_Image);

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool Animable::Destroy ()
    {
        if (m_Image != nullptr)
        {
            al_destroy_bitmap (m_Image);
            m_Image = nullptr;
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool Animable::Update (float deltaTime)
    {
        if (!m_Animation.GetAnimations ().empty ())
        {
            m_Animation.Update (deltaTime);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Animable::Render (Transformable* transformable)
    {
        if (m_Image)
        {
            float sourceX = 0;
            float sourceY = 0;
            float sourceWidth;
            float sourceHeight;

            if (m_Animation.GetAnimations ().empty ())
            {
                sourceWidth = m_ImageWidth;
                sourceHeight = m_ImageHeight;
            }
            else
            {
                AnimationFrames& frames = m_Animation.GetCurrentAnimation ();
                Rect& frame = frames.GetFrame (m_Animation.GetCurrentFrame ());

                sourceX = frame.GetPos ().X;
                sourceY = frame.GetPos ().Y;
                sourceWidth = frame.GetSize ().Width;
                sourceHeight = frame.GetSize ().Height;
            }

            Point pos = transformable->Bounds.GetPos ();

            al_draw_tinted_scaled_rotated_bitmap_region (m_Image, sourceX, sourceY, sourceWidth, sourceHeight,
                                                         al_map_rgb (255, 255, 255), sourceWidth * 0.5,
                                                         sourceHeight * 0.5, pos.X + sourceWidth * 0.5f,
                                                         pos.Y + sourceHeight * 0.5f, 1, 1, transformable->Rotation, 0);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Animable::SetCurrentAnimation (const std::string& name) { m_Animation.SetCurrentAnimation (name); }

    //--------------------------------------------------------------------------------------------------

    ALLEGRO_BITMAP* Animable::GetImage () { return m_Image; }

    //--------------------------------------------------------------------------------------------------

    std::map<std::string, AnimationFrames>& Animable::GetAnimations () { return m_Animation.GetAnimations (); }

    //--------------------------------------------------------------------------------------------------
}
