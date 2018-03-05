// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "TweenManager.h"
#include "Common.h"
#include "MainLoop.h"
#include "Screen.h"
#include "Script.h"

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

    bool TweenManager::Initialize () { return Lifecycle::Initialize (); }

    //--------------------------------------------------------------------------------------------------

    bool TweenManager::Destroy () { return Lifecycle::Destroy (); }

    //--------------------------------------------------------------------------------------------------

    bool TweenManager::Update (float deltaTime)
    {
        CleanupFinishedTweens ();

        for (int i = 0; i < m_Tweens.size (); ++i)
        {
            TweenData& tween = m_Tweens[i];

            if (!tween.IsPaused)
            {
                if (tween.TweenMask & TWEEN_F)
                {
                    if (tween.TweenF.progress () < 1.0f)
                    {
                        tween.TweenF.step ((int)(deltaTime * 1000));
                    }
                }

                if (tween.TweenMask & TWEEN_FF)
                {
                    if (tween.TweenFF.progress () < 1.0f)
                    {
                        tween.TweenFF.step ((int)(deltaTime * 1000));
                    }
                }
            }
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void TweenManager::CleanupFinishedTweens ()
    {
        for (int i = 0; i < m_Tweens.size (); ++i)
        {
            TweenData& tween = m_Tweens[i];

            if (tween.TweenMask & TWEEN_F)
            {
                if (tween.TweenF.progress () >= 1.0f)
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
                    }

                    m_Tweens.erase (m_Tweens.begin () + i);
                }
            }

            if (tween.TweenMask & TWEEN_FF)
            {
                if (tween.TweenFF.progress () >= 1.0f)
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
                    }

                    m_Tweens.erase (m_Tweens.begin () + i);
                }
            }
        }
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

    TweenData& TweenManager::AddTween (int id, float from, float to, int during, std::function<bool(float)> func)
    {
        TweenData* foundTween = GetTween (id);

        if (foundTween)
        {
            return *foundTween;
        }

        tweeny::tween<float> tween = tweeny::from (from).to (to).during (during).onStep (func);

        TweenData tweenData;
        tweenData.ID = id;
        tweenData.TweenF = tween;
        tweenData.TweenMask |= TWEEN_F;

        m_Tweens.push_back (tweenData);

        return m_Tweens.back ();
    }

    //--------------------------------------------------------------------------------------------------

    TweenData& TweenManager::AddTween (int id, tweeny::tween<float>& func)
    {
        TweenData* foundTween = GetTween (id);

        if (foundTween)
        {
            return *foundTween;
        }

        TweenData tweenData;
        tweenData.ID = id;
        tweenData.TweenF = func;
        tweenData.TweenMask |= TWEEN_F;

        m_Tweens.push_back (tweenData);

        return m_Tweens.back ();
    }

    //--------------------------------------------------------------------------------------------------

    void TweenManager::AddTween (int id, Point from, Point to, int during, asIScriptFunction* asFunc,
                                 asIScriptFunction* finishFunc)
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
                      TweenData* tweenData = FindTweenData (t);

                      if (!tweenData || !tweenData->CallbackFunc)
                      {
                          return true;
                      }

                      asIScriptFunction* callback = tweenData->CallbackFunc;
                      Point p = { x, y };

                      const char* moduleName = callback->GetModuleName ();
                      Script* script = m_MainLoop->GetScriptManager ().GetScriptByModuleName (moduleName);

                      if (script)
                      {
                          asIScriptContext* ctx = script->GetContext ();
                          ctx->Prepare (callback);
                          ctx->SetArgDWord (0, tweenData->ID);
                          ctx->SetArgFloat (1, t.progress ());
                          ctx->SetArgObject (2, &p);

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
            tweenData.TweenFF = tween;
            tweenData.TweenMask |= TWEEN_FF;

            m_Tweens.push_back (tweenData);
        }
    }

    //--------------------------------------------------------------------------------------------------

    TweenData* TweenManager::FindTweenData (tweeny::tween<float, float>& t)
    {
        for (std::vector<TweenData>::iterator it = m_Tweens.begin (); it != m_Tweens.end (); ++it)
        {
            if (it->TweenMask & TWEEN_FF)
            {
                //  Very nasty - address comparison, but it works!
                if (&it->TweenFF == &t)
                {
                    return &(*it);
                }
            }
        }

        return nullptr;
    }

    //--------------------------------------------------------------------------------------------------

    void TweenManager::Clear () { m_Tweens.clear (); }

    //--------------------------------------------------------------------------------------------------

    TweenData* TweenManager::GetTween (int id)
    {
        for (std::vector<TweenData>::iterator it = m_Tweens.begin (); it != m_Tweens.end (); ++it)
        {
            if (it->ID == id)
            {
                return &(*it);
            }
        }

        return nullptr;
    }

    //--------------------------------------------------------------------------------------------------

    void TweenManager::PauseTween (int id)
    {
        TweenData* tween = GetTween (id);

        if (tween)
        {
            tween->IsPaused = true;
        }
    }

    //--------------------------------------------------------------------------------------------------

    void TweenManager::ResumeTween (int id)
    {
        TweenData* tween = GetTween (id);

        if (tween)
        {
            tween->IsPaused = false;
        }
    }

    //--------------------------------------------------------------------------------------------------
}
