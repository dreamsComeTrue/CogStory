// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __BUTTON_IMAGE_H__
#define __BUTTON_IMAGE_H__

#include "Button.h"
#include "Image.h"

namespace aga
{
    class ButtonImage : public Button
    {
    public:
        ButtonImage (UIManager* uiManager, Point pos, const std::string& imagePath = "");
        virtual ~ButtonImage ();
        bool Initialize ();
        bool Destroy ();

        void SetSize (int width, int height);
        void SetSize (Point size);

        virtual bool Update (double deltaTime);
        virtual void Render (double deltaTime);

    protected:
        Image m_Image;
    };
}

#endif //   __BUTTON_IMAGE_H__
