// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __LABEL_H__
#define __LABEL_H__

#include "Widget.h"

namespace aga
{
    class Label : public Widget
    {
    public:
        Label (Screen* screen, const std::string& text);
        virtual ~Label ();
        bool Initialize ();
        bool Destroy ();

        void SetText (const std::string& text);

        void SetTextColor (ALLEGRO_COLOR color);
        void SetBorderColor (ALLEGRO_COLOR color);

        void SetDrawBorder (bool draw);

        virtual bool Update (double deltaTime);
        virtual void ProcessEvent (ALLEGRO_EVENT* event, double deltaTime);
        virtual void Render (double deltaTime);

    protected:
        ALLEGRO_COLOR m_TextColor;
        ALLEGRO_COLOR m_BorderColor;
        bool m_DrawBorder;
        std::string m_Text;
        Point m_TextSize;
        unsigned m_FontAscent;
    };
}

#endif //   __LABEL_H__
