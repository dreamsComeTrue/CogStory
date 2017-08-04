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
      , m_WidgetFocus (nullptr)
      , m_PreviousWidgetFocus (nullptr)
      , m_LastID (-1)
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

    bool UIManager::Initialize () { Lifecycle::Initialize (); }

    //--------------------------------------------------------------------------------------------------

    bool UIManager::Destroy ()
    {
        for (Widget* widget : m_OwnQueue)
        {
            SAFE_DELETE (widget);
        }

        return Lifecycle::Destroy ();
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
        if (event->type == ALLEGRO_EVENT_MOUSE_AXES)
        {
            bool foundWidget = false;

            for (WIDGET_ITERATOR it = m_Widgets.begin (); it != m_Widgets.end (); it++)
            {
                if (InsideRect (event->mouse.x, event->mouse.y, it->second->GetBounds (true)))
                {
                    if (m_WidgetFocus != nullptr)
                    {
                        m_WidgetFocus->MouseMove (event->mouse);

                        if (m_WidgetFocus != it->second)
                        {
                            m_PreviousWidgetFocus = m_WidgetFocus;
                            m_WidgetFocus->MouseLeave (event->mouse);
                        }
                    }

                    if (m_WidgetFocus != it->second)
                    {
                        m_WidgetFocus = it->second;
                        m_WidgetFocus->MouseEnter (event->mouse);
                    }

                    foundWidget = true;
                }
            }

            if (!foundWidget)
            {
                if (m_WidgetFocus != nullptr)
                {
                    m_PreviousWidgetFocus = m_WidgetFocus;
                    m_WidgetFocus->MouseLeave (event->mouse);
                }

                m_WidgetFocus = nullptr;
            }
        }
        else if (event->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
        {
            if (m_WidgetFocus != nullptr)
            {
                m_WidgetFocus->MouseDown (event->mouse);
            }
        }
        else if (event->type == ALLEGRO_EVENT_MOUSE_BUTTON_UP)
        {
            if (m_WidgetFocus != nullptr)
            {
                m_WidgetFocus->MouseUp (event->mouse);
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void UIManager::Render (double deltaTime)
    {
        for (WIDGET_ITERATOR it = m_Widgets.begin (); it != m_Widgets.end (); it++)
        {
            if (it->second->IsVisible ())
            {
                it->second->Render (deltaTime);
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void UIManager::AddWidget (Widget* widget, int id, bool ownMemory)
    {
        if (id < 0)
        {
            id = m_LastID + 1;
        }

        m_Widgets.insert (std::make_pair (id, widget));

        if (ownMemory)
        {
            m_OwnQueue.push_back (widget);
        }

        m_LastID = id;
    }

    //--------------------------------------------------------------------------------------------------

    Screen* UIManager::GetScreen () { return m_Screen; }

    //--------------------------------------------------------------------------------------------------

    Widget* UIManager::GetWidgetFocus () { return m_WidgetFocus; }

    //--------------------------------------------------------------------------------------------------

    Widget* UIManager::GetPreviousWidgetFocus () { return m_PreviousWidgetFocus; }

    //--------------------------------------------------------------------------------------------------
}
