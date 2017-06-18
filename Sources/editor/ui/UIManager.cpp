// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "UIManager.h"
#include "Screen.h"
#include "Widget.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    typedef std::map<int, Widget*>::iterator WIDGET_ITERATOR;

    //--------------------------------------------------------------------------------------------------

    UIManager::UIManager (Screen* screen)
        : m_Screen (screen)
    {
    }

    //--------------------------------------------------------------------------------------------------

    UIManager::~UIManager ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    bool UIManager::Initialize ()
    {
        Lifecycle::Initialize ();
    }

    //--------------------------------------------------------------------------------------------------

    bool UIManager::Destroy ()
    {
        for (Widget* widget : m_OwnQueue)
        {
            SAFE_DELETE (widget);
        }

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    bool UIManager::Update (double deltaTime)
    {
        for (WIDGET_ITERATOR it = m_Widgets.begin (); it != m_Widgets.end (); it++)
        {
            it->second->Update (deltaTime);
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void UIManager::ProcessEvent (ALLEGRO_EVENT* event, double deltaTime)
    {
        for (WIDGET_ITERATOR it = m_Widgets.begin (); it != m_Widgets.end (); it++)
        {
            it->second->ProcessEvent (event, deltaTime);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void UIManager::Render (double deltaTime)
    {
        for (WIDGET_ITERATOR it = m_Widgets.begin (); it != m_Widgets.end (); it++)
        {
            it->second->Render (deltaTime);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void UIManager::AddWidget (int id, Widget* widget, bool ownMemory)
    {
        m_Widgets.insert (std::make_pair (id, widget));

        if (ownMemory)
        {
            m_OwnQueue.push_back (widget);
        }
    }

    //--------------------------------------------------------------------------------------------------
}
