// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __UIMANAGER_H__
#define __UIMANAGER_H__

#include "Common.h"

namespace aga
{
    class Screen;
    class Widget;

    class UIManager : public Lifecycle
    {
      public:
        UIManager (Screen* screen);
        virtual ~UIManager ();
        bool Initialize ();
        bool Destroy ();

        bool Update (double deltaTime);
        void ProcessEvent (ALLEGRO_EVENT* event, double deltaTime);
        void Render (double deltaTime);

        void AddWidget (Widget* widget, int id = -1, bool ownMemory = true);

      private:
        Screen* m_Screen;
        Widget* m_WidgetFocus;
        std::map<int, Widget*> m_Widgets;
        std::vector<Widget*> m_OwnQueue;
        int m_LastID;
    };
}

#endif //   __UIMANAGER_H__
