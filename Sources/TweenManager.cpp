// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "TweenManager.h"
#include "Common.h"
#include "MainLoop.h"
#include "Screen.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    TweenManager::TweenManager (MainLoop* mainLoop)
        : m_MainLoop (mainLoop)
    {
    }

    //--------------------------------------------------------------------------------------------------

    TweenManager::~TweenManager ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    bool TweenManager::Initialize () { Lifecycle::Initialize (); }

    //--------------------------------------------------------------------------------------------------

    bool TweenManager::Destroy () { Lifecycle::Destroy (); }

    //--------------------------------------------------------------------------------------------------

    void TweenManager::AddTween (int id, int from, int to, int during, std::function<bool(int)> func)
    {
        if (m_Tweens.find (id) == m_Tweens.end ())
        {
            tweeny::tween<int> tween = tweeny::from (from).to (to).during (during).onStep (func);

            m_Tweens.insert (std::make_pair (id, tween));
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool TweenManager::Update (double deltaTime)
    {
        if (m_Tweens.size () > 0)
        {
            for (std::map<int, tweeny::tween<int> >::iterator it = m_Tweens.begin (); it != m_Tweens.end ();)
            {
                //printf ("%f\n", it->second.progress());
                if (it->second.progress () < 1)
                {
                    it->second.step ((int)(deltaTime * 1000));
                    ++it;
                }
                else
                {
                    m_Tweens.erase (it++);
                }
            }
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    MainLoop* TweenManager::GetMainLoop () { return m_MainLoop; }

    //--------------------------------------------------------------------------------------------------
}
