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
        //        if (m_Tweens.find (id) == m_Tweens.end ())
        //        {
        //            tweeny::tween<float> tween = tweeny::from (from).to (to).during (during).onStep (func);

        //            m_Tweens.insert (std::make_pair (id, tween));
        //        }
    }

    //--------------------------------------------------------------------------------------------------

    bool TweenManager::Update (float deltaTime)
    {
        for (int i = 0; i < m_Tweens.size (); ++i)
        {
            TweenData& tween = m_Tweens[i];

            if (tween.Tween.progress () >= 1.0f)
            {
                if (tween.FinishFunc)
                {
                    const char* moduleName = tween.FinishFunc->GetModuleName ();
                    Script* script = m_MainLoop->GetScriptManager ().GetScriptByModuleName (moduleName);

                    if (script)
                    {
                        asIScriptContext* ctx = script->GetContext ();
                        ctx->Prepare (tween.FinishFunc);
                        ctx->SetArgDWord (0, (int)tween.ID);
                        ctx->Execute ();
                    }

                    m_Tweens.erase (m_Tweens.begin () + i);
                }
            }
        }

        for (int i = 0; i < m_Tweens.size (); ++i)
        {
            TweenData& tween = m_Tweens[i];

            if (tween.Tween.progress () < 1.0f)
            {
                tween.Tween.step ((int)(deltaTime * 1000));
            }
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    MainLoop* TweenManager::GetMainLoop () { return m_MainLoop; }

    //--------------------------------------------------------------------------------------------------

    // void TweenManager::AddTween (int id, float from, float to, int during, asIScriptFunction* asFunc)
    //  {
    //        m_Callbacks.insert (std::make_pair (id, asFunc));

    //        std::function<bool(tweeny::tween<float> & t, float)> func = [&](tweeny::tween<float>& t, float i) {
    //            asIScriptContext* ctx = m_MainLoop->GetScriptManager ().GetEngine ()->CreateContext ();

    //            int callbackID = -1;
    //            for (std::map<int, tweeny::tween<float>>::iterator it = m_Tweens.begin (); it != m_Tweens.end ();
    //            ++it)
    //            {
    //                //  Very nasty - address comparison, but it works!
    //                if (&it->second == &t)
    //                {
    //                    callbackID = it->first;
    //                    break;
    //                }
    //            }

    //            if (callbackID < 0)
    //            {
    //                return true;
    //            }

    //            ctx->Prepare (m_Callbacks[callbackID]);
    //            ctx->SetArgFloat (0, t.progress ());
    //            ctx->SetArgFloat (1, i);

    //            int r = ctx->Execute ();

    //            asDWORD ret = 0;
    //            if (r == asEXECUTION_FINISHED)
    //            {
    //                ret = ctx->GetReturnDWord ();
    //            }

    //            ctx->Release ();

    //            return (bool)ret;
    //        };

    //        if (m_Tweens.find (id) == m_Tweens.end ())
    //        {
    //            tweeny::tween<float> tween = tweeny::from (from).to (to).during (during).onStep (func);

    //            m_Tweens.insert (std::make_pair (id, tween));
    //        }
    //  }

    //--------------------------------------------------------------------------------------------------

    void TweenManager::AddTween (
        int id, Point from, Point to, int during, asIScriptFunction* asFunc, asIScriptFunction* finishFunc)
    {
        bool found = false;

        for (std::vector<TweenData>::iterator it = m_Tweens.begin (); it != m_Tweens.end (); ++it)
        {
            if (it->ID == id)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            std::function<bool(tweeny::tween<float, float> & t, float, float)> func
                = [&](tweeny::tween<float, float>& t, float x, float y) {
                      asIScriptFunction* callback = FindCallback (t);

                      if (!callback)
                      {
                          return true;
                      }

                      Point p = { x, y };

                      const char* moduleName = callback->GetModuleName ();
                      Script* script = m_MainLoop->GetScriptManager ().GetScriptByModuleName (moduleName);

                      if (script)
                      {
                          asIScriptContext* ctx = script->GetContext ();
                          ctx->Prepare (callback);
                          ctx->SetArgFloat (0, t.progress ());
                          ctx->SetArgObject (1, &p);

                          int r = ctx->Execute ();

                          asDWORD ret = 0;
                          if (r == asEXECUTION_FINISHED)
                          {
                              ret = ctx->GetReturnDWord ();
                          }

                          return (bool)ret;
                      }

                      return false;
                  };

            tweeny::tween<float, float> tween
                = tweeny::from (from.X, from.Y).to (to.X, to.Y).during (during).onStep (func);

            TweenData tweenData;
            tweenData.ID = id;
            tweenData.CallbackFunc = asFunc;
            tweenData.FinishFunc = finishFunc;
            tweenData.Tween = tween;

            m_Tweens.push_back (tweenData);
        }
    }

    //--------------------------------------------------------------------------------------------------

    asIScriptFunction* TweenManager::FindCallback (tweeny::tween<float, float>& t)
    {
        for (std::vector<TweenData>::iterator it = m_Tweens.begin (); it != m_Tweens.end (); ++it)
        {
            //  Very nasty - address comparison, but it works!
            if (&it->Tween == &t)
            {
                return it->CallbackFunc;
            }
        }

        return nullptr;
    }

    //--------------------------------------------------------------------------------------------------

    void TweenManager::Clear () { m_Tweens.clear (); }

    //--------------------------------------------------------------------------------------------------
}
