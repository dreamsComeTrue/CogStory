// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Frame.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    Frame::Frame (Screen* screen, Rect rect, bool filled, float thickness)
      : Widget (screen, rect.TopLeft)
      , m_Rect (rect)
      , m_Filled (filled)
      , m_Thickness (thickness)
    {
    }

    //--------------------------------------------------------------------------------------------------

    Frame::~Frame ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    bool Frame::Initialize ()
    {
        Lifecycle::Initialize ();
        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool Frame::Destroy ()
    {
        Lifecycle::Destroy ();

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool Frame::Update (double deltaTime)
    {
        Widget::Update (deltaTime);
        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void Frame::Render (double deltaTime)
    {
        Widget::Render (deltaTime);

        if (m_Filled)
        {
            al_draw_filled_rectangle (m_Rect.TopLeft.X,
                                      m_Rect.TopLeft.Y,
                                      m_Rect.TopLeft.X + m_Rect.BottomRight.Width,
                                      m_Rect.TopLeft.Y + m_Rect.BottomRight.Height,
                                      m_BackGroundColor);
        }
        else
        {
            al_draw_rectangle (m_Rect.TopLeft.X,
                               m_Rect.TopLeft.Y,
                               m_Rect.TopLeft.X + m_Rect.BottomRight.Width,
                               m_Rect.TopLeft.Y + m_Rect.BottomRight.Height,
                               m_BackGroundColor,
                               0.0);
        }

        if (m_DrawBorder)
        {
            al_draw_rectangle (m_Rect.TopLeft.X,
                               m_Rect.TopLeft.Y,
                               m_Rect.TopLeft.X + m_Rect.BottomRight.Width,
                               m_Rect.TopLeft.Y + m_Rect.BottomRight.Height,
                               m_BorderColor,
                               m_Thickness);
        }
    }

    //--------------------------------------------------------------------------------------------------
}
