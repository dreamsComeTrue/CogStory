// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "ButtonImage.h"
#include "Screen.h"
#include "UIManager.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    ButtonImage::ButtonImage (UIManager* uiManager, Point pos, const std::string& imagePath)
      : Button (uiManager, pos, "")
      , m_Image (uiManager, pos, imagePath)
    {
        m_Image.SetDrawWithTint (true);

        SetBackgroundColor (COLOR_WHITE);
        SetSize (m_Image.GetImageSize ());
    }

    //--------------------------------------------------------------------------------------------------

    ButtonImage::~ButtonImage ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    bool ButtonImage::Initialize () { Lifecycle::Initialize (); }

    //--------------------------------------------------------------------------------------------------

    bool ButtonImage::Destroy ()
    {
        m_Image.Destroy ();

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    void ButtonImage::SetSize (int width, int height)
    {
        Button::SetSize (width, height);
        m_Image.SetSize (width, height);
    }

    //--------------------------------------------------------------------------------------------------

    void ButtonImage::SetSize (Point size)
    {
        Button::SetSize (size);
        m_Image.SetSize (size);
    }

    //--------------------------------------------------------------------------------------------------

    bool ButtonImage::Update (double deltaTime)
    {
        Button::Update (deltaTime);

        if (m_IsPressed || m_IsHighlight)
        {
            m_Image.SetDrawWithTint (true);
        }
        else
        {
            m_Image.SetDrawWithTint (false);
        }

        ALLEGRO_COLOR color = m_IsPressed ? m_PressedColor : m_IsHighlight ? m_HighlightColor : m_BackGroundColor;

        m_Image.SetTintColor (color);

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void ButtonImage::Render (double deltaTime) { m_Image.Render (deltaTime); }

    //--------------------------------------------------------------------------------------------------
}
