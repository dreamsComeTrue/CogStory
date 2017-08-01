// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Menu.h"
#include "Screen.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    MenuItem::MenuItem (const std::string& name, Menu* menu)
        : Button (menu->GetScreen (), Point (), name)
        , m_Menu (menu)
    {
        SetBackgroundColor (COLOR_GRAY);
        SetTextColor (COLOR_WHITE);
        SetBorderColor (COLOR_BLACK);
        SetDrawBorder (true);

        m_Menu->AddItem (this);
    }

    //--------------------------------------------------------------------------------------------------

    MenuItem::~MenuItem ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------
    bool MenuItem::Initialize () { Lifecycle::Initialize (); }

    //--------------------------------------------------------------------------------------------------

    bool MenuItem::Destroy () { Lifecycle::Destroy (); }

    //--------------------------------------------------------------------------------------------------

    void MenuItem::AddChild (MenuItem* item) {}

    //--------------------------------------------------------------------------------------------------

    bool MenuItem::Update (double deltaTime) { return Button::Update (deltaTime); }

    //--------------------------------------------------------------------------------------------------

    void MenuItem::Render (double deltaTime) { Button::Render (deltaTime); }

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    Menu::Menu (Screen* screen, Point pos)
        : Widget (screen, pos)
    {
    }

    //--------------------------------------------------------------------------------------------------

    Menu::~Menu ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    bool Menu::Initialize () { Lifecycle::Initialize (); }

    //--------------------------------------------------------------------------------------------------

    bool Menu::Destroy () { Lifecycle::Destroy (); }

    //--------------------------------------------------------------------------------------------------

    void Menu::AddItem (MenuItem* item)
    {
        if (m_Items.find (item->GetText ()) == m_Items.end ())
        {
            item->Initialize ();
            m_Items.insert (std::make_pair (item->GetText (), item));
            m_ItemsList.push_back (item);
        }

        Point offset = GetPosition ();

        int itemPadding = 2;

        for (int i = 0; i < m_ItemsList.size (); ++i)
        {
            MenuItem* item = m_ItemsList[i];
            item->SetPosition (offset.X, offset.Y);

            if (i < m_ItemsList.size () - 1)
            {
                MenuItem* nextItem = m_ItemsList[i + 1];
                Rect nextBounds = nextItem->GetBounds (true);

                offset.X += nextBounds.BottomRight.Width + itemPadding;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool Menu::Update (double deltaTime)
    {
        for (std::map<std::string, MenuItem*>::iterator it = m_Items.begin (); it != m_Items.end (); ++it)
        {
            it->second->Update (deltaTime);
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void Menu::Render (double deltaTime)
    {
        for (std::map<std::string, MenuItem*>::iterator it = m_Items.begin (); it != m_Items.end (); ++it)
        {
            it->second->Render (deltaTime);
        }
    }

    //--------------------------------------------------------------------------------------------------
}
