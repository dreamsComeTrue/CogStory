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
    }

    //--------------------------------------------------------------------------------------------------

    bool TweenManager::Initialize () { Lifecycle::Initialize (); }

    //--------------------------------------------------------------------------------------------------

    bool TweenManager::Destroy () { Lifecycle::Destroy (); }

    //--------------------------------------------------------------------------------------------------

    void TweenManager::AddTween (int id, float from, float to, int during, std::function<bool(float)> func)
    {
        if (m_Tweens.find (id) == m_Tweens.end ())
        {
            tweeny::tween<float> tween = tweeny::from (from).to (to).during (during).onStep (func);

            m_Tweens.insert (std::make_pair (id, tween));
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool TweenManager::Update (float deltaTime)
    {
        if (m_Tweens.size () > 0)
        {
            for (std::map<int, tweeny::tween<float>>::iterator it = m_Tweens.begin (); it != m_Tweens.end ();)
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

        if (m_Tweens2.size () > 0)
        {
            for (std::map<int, tweeny::tween<float, float>>::iterator it = m_Tweens2.begin (); it != m_Tweens2.end ();)
            {
                if (it->second.progress () < 1)
                {
                    it->second.step ((int)(deltaTime * 1000));
                    ++it;
                }
                else
                {
                    m_Tweens2.erase (it++);
                }
            }
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    MainLoop* TweenManager::GetMainLoop () { return m_MainLoop; }

    //--------------------------------------------------------------------------------------------------

    void TweenManager::AddTween (int id, float from, float to, int during, asIScriptFunction* asFunc)
    {
        m_Callbacks.insert (std::make_pair (id, asFunc));

        std::function<bool(tweeny::tween<float> & t, float)> func = [&](tweeny::tween<float>& t, float i) {
            asIScriptContext* ctx = m_MainLoop->GetScriptManager ().GetEngine ()->CreateContext ();

            int callbackID = -1;
            for (std::map<int, tweeny::tween<float>>::iterator it = m_Tweens.begin (); it != m_Tweens.end (); ++it)
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
            ctx->SetArgFloat (0, t.progress ());
            ctx->SetArgFloat (1, i);

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
            tweeny::tween<float> tween = tweeny::from (from).to (to).during (during).onStep (func);

            m_Tweens.insert (std::make_pair (id, tween));
        }
    }

    //--------------------------------------------------------------------------------------------------

    void TweenManager::AddTween (int id, Point from, Point to, int during, asIScriptFunction* asFunc)
    {
        m_Callbacks.insert (std::make_pair (id, asFunc));

        std::function<bool(tweeny::tween<float, float> & t, float, float)> func
            = [&](tweeny::tween<float, float>& t, float x, float y) {
                  asIScriptContext* ctx = m_MainLoop->GetScriptManager ().GetEngine ()->CreateContext ();

                  int callbackID = -1;
                  for (std::map<int, tweeny::tween<float, float>>::iterator it = m_Tweens2.begin ();
                       it != m_Tweens2.end (); ++it)
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

                  Point p = { x, y };

                  ctx->Prepare (m_Callbacks[callbackID]);
                  ctx->SetArgFloat (0, t.progress ());
                  ctx->SetArgObject (1, &p);

                  int r = ctx->Execute ();

                  asDWORD ret = 0;
                  if (r == asEXECUTION_FINISHED)
                  {
                      ret = ctx->GetReturnDWord ();
                  }

                  ctx->Release ();

                  return (bool)ret;
              };

        if (m_Tweens2.find (id) == m_Tweens2.end ())
        {
            tweeny::tween<float, float> tween
                = tweeny::from (from.X, from.Y).to (to.X, to.Y).during (during).onStep (func);

            m_Tweens2.insert (std::make_pair (id, tween));
        }
    }

    //--------------------------------------------------------------------------------------------------
}
