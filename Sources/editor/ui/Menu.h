// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __MENU_H__
#define __MENU_H__

#include "Button.h"

namespace aga
{
    class Menu;

    class MenuItem : public Button
    {
    public:
        MenuItem (const std::string& name, Menu* menu);
        virtual ~MenuItem ();
        bool Initialize ();
        bool Destroy ();

        void AddChild (MenuItem* item);

        virtual bool Update (double deltaTime);
        virtual void Render (double deltaTime);

    private:
        Menu* m_Menu;
    };

    class Menu : public Widget
    {
    public:
        Menu (Screen* screen, Point pos);
        virtual ~Menu ();
        bool Initialize ();
        bool Destroy ();

        void AddItem (MenuItem* item);

        virtual bool Update (double deltaTime);
        virtual void Render (double deltaTime);

    private:
        std::vector<MenuItem*> m_ItemsList;
        std::map<std::string, MenuItem*> m_Items;
    };
}

#endif //   __MENU_H__
