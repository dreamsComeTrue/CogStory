// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __BUTTON_H__
#define __BUTTON_H__

#include "Label.h"

namespace aga
{
    class Button : public Label
    {
    public:
        Button (Screen* screen, const std::string& text = "");
        virtual ~Button ();
        bool Initialize ();
        bool Destroy ();

        virtual bool Update (double deltaTime);
        virtual void ProcessEvent (ALLEGRO_EVENT* event, double deltaTime);
        virtual void Render (double deltaTime);
    };
}

#endif //   __BUTTON_H__
