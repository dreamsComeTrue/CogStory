// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Image.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    Image::Image (Screen* screen, Point pos, const std::string& imagePath)
      : Widget (screen, pos)
      , m_DrawWithTint (false)
      , m_TintColor (COLOR_WHITE)
    {
        SetImagePath (imagePath);
    }

    //--------------------------------------------------------------------------------------------------

    Image::~Image ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    bool Image::Initialize () { Lifecycle::Initialize (); }

    //--------------------------------------------------------------------------------------------------

    bool Image::Destroy ()
    {
        al_destroy_bitmap (m_Bitmap);

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    bool Image::Update (double deltaTime)
    {
        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void Image::SetImagePath (const std::string& imagePath)
    {
        m_ImagePath = imagePath;
        m_Bitmap = al_load_bitmap (imagePath.c_str ());
        m_Size = { al_get_bitmap_width (m_Bitmap), al_get_bitmap_height (m_Bitmap) };
        m_Bounds.BottomRight = { m_Bounds.TopLeft.X + m_Size.X, m_Bounds.TopLeft.Y + m_Size.Y };
    }

    //--------------------------------------------------------------------------------------------------

    Point Image::GetImageSize () { return m_Size; }

    //--------------------------------------------------------------------------------------------------

    void Image::SetDrawWithTint (bool enable) { m_DrawWithTint = enable; }

    //--------------------------------------------------------------------------------------------------

    void Image::SetTintColor (ALLEGRO_COLOR color) { m_TintColor = color; }

    //--------------------------------------------------------------------------------------------------

    void Image::Render (double deltaTime)
    {
        if (m_DrawWithTint)
        {
            al_draw_tinted_scaled_bitmap (m_Bitmap,
                                          m_TintColor,
                                          0,
                                          0,
                                          m_Size.Width,
                                          m_Size.Height,
                                          m_Bounds.TopLeft.X,
                                          m_Bounds.TopLeft.Y,
                                          m_Bounds.BottomRight.Width - m_Bounds.TopLeft.X,
                                          m_Bounds.BottomRight.Height - m_Bounds.TopLeft.Y,
                                          0);
        }
        else
        {

            al_draw_scaled_bitmap (m_Bitmap,
                                   0,
                                   0,
                                   m_Size.Width,
                                   m_Size.Height,
                                   m_Bounds.TopLeft.X,
                                   m_Bounds.TopLeft.Y,
                                   m_Bounds.BottomRight.Width - m_Bounds.TopLeft.X,
                                   m_Bounds.BottomRight.Height - m_Bounds.TopLeft.Y,
                                   0);
        }
    }

    //--------------------------------------------------------------------------------------------------
}
