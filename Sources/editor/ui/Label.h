// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __LABEL_H__
#define __LABEL_H__

#include "Widget.h"

namespace aga
{
    class Label : public Widget
    {
    public:
        Label (Screen* screen, Point pos, const std::string& text);
        virtual ~Label ();
        bool Initialize ();
        bool Destroy ();

        void SetText (const std::string& text);
        void SetTextColor (ALLEGRO_COLOR color);

        virtual bool Update (double deltaTime);
        virtual void Render (double deltaTime);

    protected:
        ALLEGRO_COLOR m_TextColor;
        std::string m_Text;
        Point m_TextSize;
        unsigned m_FontAscent;
    };
}

#endif //   __LABEL_H__
