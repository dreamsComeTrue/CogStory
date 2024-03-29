// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "TweenManager.h"
#include "MainLoop.h"
#include "Timeline.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    static int NEXT_TWEEN_ID = 1;

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
        for (size_t i = 0; i < m_Tweens.size (); ++i)
        {
            TweenData& tween = m_Tweens[i];

            if (!tween.IsPaused)
            {
                if (tween.TweenMask & TWEEN_F)
                {
                    if (tween.BeginScriptFunc && tween.TweenF.progress () <= 0.0f)
                    {
                        asIScriptContext* ctx = m_MainLoop->GetScriptManager ().GetContext ();
                        ctx->Prepare (tween.BeginScriptFunc);
                        ctx->SetArgDWord (0, static_cast<asDWORD> (tween.ID));
                        ctx->Execute ();
                        ctx->Unprepare ();
                        ctx->GetEngine ()->ReturnContext (ctx);
                    }

                    tween.TweenF.step (static_cast<int> (deltaTime * 1000));
                }

                if (tween.TweenMask & TWEEN_FF)
                {
                    if (tween.BeginScriptFunc && tween.TweenFF.progress () <= 0.0f)
                    {
                        asIScriptContext* ctx = m_MainLoop->GetScriptManager ().GetContext ();
                        ctx->Prepare (tween.BeginScriptFunc);
                        ctx->SetArgDWord (0, static_cast<asDWORD> (tween.ID));
                        ctx->Execute ();
                        ctx->Unprepare ();
                        ctx->GetEngine ()->ReturnContext (ctx);
                    }

                    tween.TweenFF.step (static_cast<int> (deltaTime * 1000));
                }
            }
        }

        for (size_t i = 0; i < m_Timelines.size (); ++i)
        {
            if (m_Timelines[i])
            {
                m_Timelines[i]->Update (deltaTime);
            }
        }

        CleanupFinishedTweens ();

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void TweenManager::CleanupFinishedTweens ()
    {
        for (size_t i = 0; i < m_Tweens.size (); ++i)
        {
            if (((m_Tweens[i].TweenMask & TWEEN_F) && (m_Tweens[i].TweenF.progress () >= 1.0f))
                || ((m_Tweens[i].TweenMask & TWEEN_FF) && (m_Tweens[i].TweenFF.progress () >= 1.0f)))
            {
                TweenData& tweenToProcess = m_Tweens[i];

                if (tweenToProcess.FinishFunc)
                {
                    tweenToProcess.FinishFunc (tweenToProcess.ID);
                }

                if (tweenToProcess.FinishScriptFunc)
                {
                    asIScriptContext* ctx = m_MainLoop->GetScriptManager ().GetContext ();
                    ctx->Prepare (tweenToProcess.FinishScriptFunc);
                    ctx->SetArgDWord (0, static_cast<asDWORD> (tweenToProcess.ID));
                    ctx->Execute ();
                    ctx->Unprepare ();
                    ctx->GetEngine ()->ReturnContext (ctx);
                }

                m_Tweens.erase (m_Tweens.begin () + i);
            }
        }
    }

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

    TweenData& TweenManager::AddTween (int id, float from, float to, int during,
        std::function<bool(float)> callbackFunction, std::function<void(int)> finishFunction)
    {
        TweenData* foundTween = GetTween (id);

        if (foundTween)
        {
            return *foundTween;
        }

        tweeny::tween<float> tween = tweeny::from (from).to (to).during (during).onStep (callbackFunction);

        TweenData tweenData;
        tweenData.ID = GetNextTweenID (id);
        tweenData.TweenF = tween;
        tweenData.TweenMask |= TWEEN_F;
        tweenData.FinishFunc = finishFunction;

        m_Tweens.push_back (tweenData);

        return m_Tweens.back ();
    }

    //--------------------------------------------------------------------------------------------------

    TweenData& TweenManager::AddTween (
        int id, tweeny::tween<float>& callbackFunction, std::function<void(int)> finishFunction)
    {
        TweenData* foundTween = GetTween (id);

        if (foundTween)
        {
            return *foundTween;
        }

        TweenData tweenData;
        tweenData.ID = GetNextTweenID (id);
        tweenData.TweenF = callbackFunction;
        tweenData.TweenMask |= TWEEN_F;
        tweenData.FinishFunc = finishFunction;

        m_Tweens.push_back (tweenData);

        return m_Tweens.back ();
    }

    //--------------------------------------------------------------------------------------------------

    TweenData& TweenManager::AddTween (int id, Point from, Point to, int during,
        std::function<bool(float, float)> callbackFunction, std::function<void(int)> finishFunction)
    {
        TweenData* foundTween = GetTween (id);

        if (foundTween)
        {
            return *foundTween;
        }

        tweeny::tween<float, float> tween
            = tweeny::from (from.X, from.Y).to (to.X, to.Y).during (during).onStep (callbackFunction);

        TweenData tweenData;
        tweenData.ID = GetNextTweenID (id);
        tweenData.TweenFF = tween;
        tweenData.TweenMask |= TWEEN_FF;
        tweenData.FinishFunc = finishFunction;

        m_Tweens.push_back (tweenData);

        return m_Tweens.back ();
    }

    //--------------------------------------------------------------------------------------------------

    TweenData& TweenManager::AddTween (int id, Point from, Point to, int during,
        std::function<bool(float, float)> callbackFunction, asIScriptFunction* finishFunc)
    {
        TweenData* foundTween = GetTween (id);

        if (foundTween)
        {
            return *foundTween;
        }

        tweeny::tween<float, float> tween
            = tweeny::from (from.X, from.Y).to (to.X, to.Y).during (during).onStep (callbackFunction);

        TweenData tweenData;
        tweenData.ID = GetNextTweenID (id);
        tweenData.FinishScriptFunc = finishFunc;
        tweenData.TweenFF = tween;
        tweenData.TweenMask |= TWEEN_FF;

        m_Tweens.push_back (tweenData);

        return m_Tweens.back ();
    }

    //--------------------------------------------------------------------------------------------------

    int TweenManager::AddTween (
        Point from, Point to, int during, asIScriptFunction* asFunc, asIScriptFunction* finishFunc)
    {
        std::function<bool(tweeny::tween<float, float> & t, float, float)> func
            = [&](tweeny::tween<float, float>& t, float x, float y) {
                  TweenData* tweenData = FindTweenData (t);

                  if (!tweenData || !tweenData->CallbackScriptFunc)
                  {
                      return true;
                  }

                  Point p = {x, y};

                  asIScriptContext* ctx = m_MainLoop->GetScriptManager ().GetContext ();
                  ctx->Prepare (tweenData->CallbackScriptFunc);
                  ctx->SetArgDWord (0, static_cast<asDWORD> (tweenData->ID));
                  ctx->SetArgFloat (1, t.progress ());
                  ctx->SetArgObject (2, &p);

                  int r = ctx->Execute ();

                  asDWORD ret = 0;
                  if (r == asEXECUTION_FINISHED)
                  {
                      ret = ctx->GetReturnDWord ();
                  }

                  ctx->Unprepare ();
                  ctx->GetEngine ()->ReturnContext (ctx);

                  return static_cast<bool> (ret);
              };

        tweeny::tween<float, float> tween = tweeny::from (from.X, from.Y).to (to.X, to.Y).during (during).onStep (func);

        TweenData tweenData;
        tweenData.ID = GetNextTweenID ();
        tweenData.CallbackScriptFunc = asFunc;
        tweenData.FinishScriptFunc = finishFunc;
        tweenData.TweenFF = tween;
        tweenData.TweenMask |= TWEEN_FF;

        m_Tweens.push_back (tweenData);

        return tweenData.ID;
    }

    //--------------------------------------------------------------------------------------------------

    void TweenManager::RemoveTween (int id)
    {
        for (size_t i = 0; i < m_Tweens.size (); ++i)
        {
            if (m_Tweens[i].ID == id)
            {
                m_Tweens.erase (m_Tweens.begin () + i);
                return;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    TweenData* TweenManager::FindTweenData (tweeny::tween<float, float>& t)
    {
        for (TweenData& tween : m_Tweens)
        {
            if (tween.TweenMask & TWEEN_FF)
            {
                //  Very nasty - address comparison, but it works!
                if (&tween.TweenFF == &t)
                {
                    return &tween;
                }
            }
        }

        return nullptr;
    }

    //--------------------------------------------------------------------------------------------------

    void TweenManager::Clear ()
    {
        m_Tweens.clear ();

        for (size_t i = 0; i < m_Timelines.size (); ++i)
        {
            SAFE_DELETE (m_Timelines[i]);
        }
    }

    //--------------------------------------------------------------------------------------------------

    TweenData* TweenManager::GetTween (int id)
    {
        for (TweenData& tween : m_Tweens)
        {
            if (tween.ID == id)
            {
                return &tween;
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

    Timeline* TweenManager::CreateTimeline (int id)
    {
        if (id < 0)
        {
            id = GetNextTweenID ();
        }
        
        Timeline* timeLine = new Timeline (id, this);

        m_Timelines.push_back (timeLine);

        return timeLine;
    }

    //--------------------------------------------------------------------------------------------------

    int TweenManager::GetNextTweenID (int id)
    {
        if (id < 0)
        {
            return NEXT_TWEEN_ID++;
        }
        else
        {
            return id;
        }
    }

    //--------------------------------------------------------------------------------------------------
}
