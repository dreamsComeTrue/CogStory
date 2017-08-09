// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Menu.h"
#include "Screen.h"
#include "UIManager.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    MenuItem::MenuItem (const std::string& name, Menu* menu)
      : Button (menu->GetUIManager (), Point (), name)
      , m_Menu (menu)
      , m_Parent (nullptr)
    {
        SetBackgroundColor (COLOR_GRAY);
        SetTextColor (COLOR_WHITE);
        SetHighlightColor ({ 0.2f, 0.2f, 0.2f, 1.0f });
        SetPressedColor ({ 0.5f, 0.5f, 0.5f, 1.0f });
        SetBorderColor (COLOR_BLACK);
        SetDrawBorder (true);
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

    void MenuItem::AddChild (MenuItem* item)
    {
        for (MenuItem* child : m_Children)
        {
            if (child == item)
            {
                return;
            }
        }

        m_Children.push_back (item);
        m_UIManager->AddWidget (item, -1, false);

        item->SetVisible (false);
        item->m_Parent = this;

        RepositionItems ();
    }

    //--------------------------------------------------------------------------------------------------

    void MenuItem::SetPosition (int x, int y)
    {
        Button::SetPosition (x, y);
        RepositionItems ();
    }

    bool MenuItem::Update (double deltaTime) { return Button::Update (deltaTime); }

    //--------------------------------------------------------------------------------------------------

    void MenuItem::Render (double deltaTime) { Button::Render (deltaTime); }

    //--------------------------------------------------------------------------------------------------

    void MenuItem::MouseEnter (ALLEGRO_MOUSE_EVENT& event)
    {
        Button::MouseEnter (event);

        if (IsMenuHeaderSelected () && m_Parent != nullptr)
        {
            m_Parent->ShowChildren ();
        }

        ShowChildren ();
    }

    //--------------------------------------------------------------------------------------------------

    void MenuItem::MouseLeave (ALLEGRO_MOUSE_EVENT& event)
    {
        Button::MouseLeave (event);

        if (m_Parent != nullptr)
        {
            m_Parent->HideChildren ();
        }

        HideChildren ();
    }

    //--------------------------------------------------------------------------------------------------

    void MenuItem::MouseUp (ALLEGRO_MOUSE_EVENT& event)
    {
        Button::MouseUp (event);

        if (m_Parent != nullptr)
        {
            m_Parent->HideChildren ();
        }

        HideChildren ();
    }

    //--------------------------------------------------------------------------------------------------

    void MenuItem::RepositionItems ()
    {
        Point offset = GetPosition ();

        for (int i = 0; i < m_Children.size (); ++i)
        {
            offset.Y += GetSize ().Height;

            MenuItem* item = m_Children[i];
            item->SetPosition (offset.X, offset.Y);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void MenuItem::ShowChildren ()
    {
        if (!m_Children.empty ())
        {
            for (MenuItem* child : m_Children)
            {
                child->SetVisible (true);
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void MenuItem::HideChildren ()
    {
        if (!m_Children.empty ())
        {
            for (MenuItem* child : m_Children)
            {
                child->SetVisible (false);
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool MenuItem::IsMenuHeaderSelected ()
    {
        Widget* prevWidget = m_UIManager->GetPreviousWidgetFocus ();

        bool found = false;
        if (m_Parent != nullptr)
        {
            for (MenuItem* child : m_Parent->m_Children)
            {
                if (child == prevWidget)
                {
                    found = true;
                    break;
                }
            }
        }

        return prevWidget == m_Parent || found;
    }

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    Menu::Menu (UIManager* uiManager, Point pos)
      : Widget (uiManager, pos)
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

            m_UIManager->AddWidget (item, -1, false);
        }

        RepositionItems ();
    }

    //--------------------------------------------------------------------------------------------------

    void Menu::RepositionItems ()
    {
        Point offset = GetPosition ();

        int itemPadding = 4;

        for (MenuItem* item : m_ItemsList)
        {
            item->SetPosition (offset.X, offset.Y);

            offset.X = item->GetPosition ().X + item->GetSize ().Width + itemPadding;
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
