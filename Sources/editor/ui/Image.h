// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "Widget.h"

namespace aga
{
    class Image : public Widget
    {
    public:
        Image (Screen* screen, Point pos, const std::string& imagePath);
        virtual ~Image ();
        bool Initialize ();
        bool Destroy ();

        void SetImagePath (const std::string& text);
        Point GetImageSize ();

        void SetDrawWithTint (bool enable = true);
        void SetTintColor (ALLEGRO_COLOR color);

        virtual bool Update (double deltaTime);
        virtual void Render (double deltaTime);

    protected:
        ALLEGRO_BITMAP* m_Bitmap;
        Point m_Size;
        std::string m_ImagePath;
        bool m_DrawWithTint;
        ALLEGRO_COLOR m_TintColor;
    };
}

#endif //   __IMAGE_H__
