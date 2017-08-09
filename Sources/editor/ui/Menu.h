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

        void SetPosition (int x, int y) override;

        virtual bool Update (double deltaTime) override;
        virtual void Render (double deltaTime) override;

        virtual void MouseEnter (ALLEGRO_MOUSE_EVENT& event) override;
        virtual void MouseLeave (ALLEGRO_MOUSE_EVENT& event) override;
        virtual void MouseUp (ALLEGRO_MOUSE_EVENT& event) override;

    private:
        void RepositionItems ();
        void ShowChildren ();
        void HideChildren ();
        bool IsMenuHeaderSelected ();

    private:
        Menu* m_Menu;
        MenuItem* m_Parent;
        std::vector<MenuItem*> m_Children;
    };

    class Menu : public Widget
    {
    public:
        Menu (UIManager* uiManager, Point pos);
        virtual ~Menu ();
        bool Initialize ();
        bool Destroy ();

        void AddItem (MenuItem* item);

        virtual bool Update (double deltaTime);
        virtual void Render (double deltaTime);

    private:
        void RepositionItems ();

    private:
        std::vector<MenuItem*> m_ItemsList;
        std::map<std::string, MenuItem*> m_Items;
    };
}

#endif //   __MENU_H__
