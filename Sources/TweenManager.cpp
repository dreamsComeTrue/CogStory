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
            for (std::map<int, tweeny::tween<int>>::iterator it = m_Tweens.begin (); it != m_Tweens.end ();)
            {
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

    void TweenManager::AddTween (int id, int from, int to, int during, asIScriptFunction* asFunc)
    {
        m_Callbacks.insert (std::make_pair (id, asFunc));

        std::function<bool(tweeny::tween<int> & t, int)> func = [&](tweeny::tween<int>& t, int i) {
            asIScriptContext* ctx = m_MainLoop->GetScriptManager ().GetEngine ()->CreateContext ();

            int callbackID = -1;
            for (std::map<int, tweeny::tween<int>>::iterator it = m_Tweens.begin (); it != m_Tweens.end (); ++it)
            {
                //  Very nasty - address comparison, but it works!
                if (&it->second == &t)
                {
                    callbackID = it->first;
                    break;
                }
            }

            if (callbackID < 0)
            {
                return true;
            }

            ctx->Prepare (m_Callbacks[callbackID]);
            ctx->SetArgDWord (0, i);

            int r = ctx->Execute ();

            asDWORD ret = 0;
            if (r == asEXECUTION_FINISHED)
            {
                ret = ctx->GetReturnDWord ();
            }

            ctx->Release ();

            return (bool)ret;
        };

        if (m_Tweens.find (id) == m_Tweens.end ())
        {
            tweeny::tween<int> tween = tweeny::from (from).to (to).during (during).onStep (func);

            m_Tweens.insert (std::make_pair (id, tween));
        }
    }

    //--------------------------------------------------------------------------------------------------
}
