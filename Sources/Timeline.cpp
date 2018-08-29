// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Timeline.h"
#include "MainLoop.h"
#include "ScriptManager.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    Timeline::Timeline (int id, TweenManager* tweenManager)
        : m_ID (id)
        , m_TweenManager (tweenManager)
        , m_CurrentTweenIndex (0)
    {
    }

    //--------------------------------------------------------------------------------------------------

    Timeline::~Timeline () {}

    //--------------------------------------------------------------------------------------------------

    bool Timeline::Update (float deltaTime)
    {
        CleanupFinishedTweens ();

        if (m_CurrentTweenIndex < m_Tweens.size ())
        {
            TweenData& tween = m_Tweens[m_CurrentTweenIndex];

            if (!tween.IsPaused)
            {
                if (tween.TweenMask & TWEEN_F)
                {
                    if (tween.BeginScriptFunc && tween.TweenF.progress () <= 0.0f)
                    {
                        asIScriptContext* ctx = m_TweenManager->GetMainLoop ()->GetScriptManager ().GetContext ();
                        ctx->Prepare (tween.BeginScriptFunc);
                        ctx->SetArgDWord (0, tween.ID);
                        ctx->Execute ();
                        ctx->Unprepare ();
                        ctx->GetEngine ()->ReturnContext (ctx);
                    }

                    tween.TweenF.step ((int)(deltaTime * 1000));
                }

                if (tween.TweenMask & TWEEN_FF)
                {
                    if (tween.BeginScriptFunc && tween.TweenFF.progress () <= 0.0f)
                    {
                        asIScriptContext* ctx = m_TweenManager->GetMainLoop ()->GetScriptManager ().GetContext ();
                        ctx->Prepare (tween.BeginScriptFunc);
                        ctx->SetArgDWord (0, tween.ID);
                        ctx->Execute ();
                        ctx->Unprepare ();
                        ctx->GetEngine ()->ReturnContext (ctx);
                    }

                    tween.TweenFF.step ((int)(deltaTime * 1000));
                }
            }
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void Timeline::CleanupFinishedTweens ()
    {
        if (m_CurrentTweenIndex < m_Tweens.size ())
        {
            if (((m_Tweens[m_CurrentTweenIndex].TweenMask & TWEEN_F)
                    && (m_Tweens[m_CurrentTweenIndex].TweenF.progress () >= 1.0f))
                || ((m_Tweens[m_CurrentTweenIndex].TweenMask & TWEEN_FF)
                       && (m_Tweens[m_CurrentTweenIndex].TweenFF.progress () >= 1.0f))
                || (m_Tweens[m_CurrentTweenIndex].TweenMask == 0))
            {
                TweenData& tweenToProcess = m_Tweens[m_CurrentTweenIndex];

                if (tweenToProcess.FinishFunc)
                {
                    tweenToProcess.FinishFunc (tweenToProcess.ID);
                }

                if (tweenToProcess.FinishScriptFunc)
                {
                    asIScriptContext* ctx = m_TweenManager->GetMainLoop ()->GetScriptManager ().GetContext ();
                    ctx->Prepare (tweenToProcess.FinishScriptFunc);
                    ctx->SetArgDWord (0, tweenToProcess.ID);
                    ctx->Execute ();
                    ctx->Unprepare ();
                    ctx->GetEngine ()->ReturnContext (ctx);
                }

                // m_Tweens.erase (m_Tweens.begin () + i);

                ++m_CurrentTweenIndex;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    Timeline* Timeline::Once (int duringMS, asIScriptFunction* asFunc)
    {
        tweeny::tween<float> tween = tweeny::from (0.f).to (1.f).during (duringMS);

        TweenData tweenData;
        tweenData.ID = m_ID;
        tweenData.BeginScriptFunc = asFunc;
        tweenData.TweenF = tween;
        tweenData.TweenMask |= TWEEN_F;

        m_Tweens.push_back (tweenData);

        return this;
    }

    //--------------------------------------------------------------------------------------------------

    Timeline* Timeline::During (float from, float to, int duringMS, asIScriptFunction* asFunc)
    {
        std::function<bool(tweeny::tween<float> & t, float)> func = [&](tweeny::tween<float>& t, float value) {
            TweenData* tweenData = FindTweenData (t);

            if (!tweenData || !tweenData->CallbackScriptFunc)
            {
                return true;
            }

            asIScriptContext* ctx = m_TweenManager->GetMainLoop ()->GetScriptManager ().GetContext ();
            ctx->Prepare (tweenData->CallbackScriptFunc);
            ctx->SetArgDWord (0, tweenData->ID);
            ctx->SetArgFloat (1, t.progress ());
            ctx->SetArgFloat (2, value);

            int r = ctx->Execute ();

            asDWORD ret = 0;
            if (r == asEXECUTION_FINISHED)
            {
                ret = ctx->GetReturnDWord ();
            }

            ctx->Unprepare ();
            ctx->GetEngine ()->ReturnContext (ctx);

            return (bool)ret;
        };

        tweeny::tween<float> tween = tweeny::from (from).to (to).during (duringMS).onStep (func);

        TweenData tweenData;
        tweenData.ID = m_ID;
        tweenData.CallbackScriptFunc = asFunc;
        tweenData.TweenF = tween;
        tweenData.TweenMask |= TWEEN_F;

        m_Tweens.push_back (tweenData);

        return this;
    }

    //--------------------------------------------------------------------------------------------------

    Timeline* Timeline::During (Point from, Point to, int duringMS, asIScriptFunction* asFunc)
    {
        std::function<bool(tweeny::tween<float, float> & t, float, float)> func
            = [&](tweeny::tween<float, float>& t, float x, float y) {
                  TweenData* tweenData = FindTweenData (t);

                  if (!tweenData || !tweenData->CallbackScriptFunc)
                  {
                      return true;
                  }

                  Point p = {x, y};

                  asIScriptContext* ctx = m_TweenManager->GetMainLoop ()->GetScriptManager ().GetContext ();
                  ctx->Prepare (tweenData->CallbackScriptFunc);
                  ctx->SetArgDWord (0, tweenData->ID);
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

                  return (bool)ret;
              };

        tweeny::tween<float, float> tween
            = tweeny::from (from.X, from.Y).to (to.X, to.Y).during (duringMS).onStep (func);

        TweenData tweenData;
        tweenData.ID = m_ID;
        tweenData.CallbackScriptFunc = asFunc;
        tweenData.TweenFF = tween;
        tweenData.TweenMask |= TWEEN_FF;

        m_Tweens.push_back (tweenData);

        return this;
    }

    //--------------------------------------------------------------------------------------------------

    TweenData* Timeline::FindTweenData (tweeny::tween<float, float>& t)
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

    TweenData* Timeline::FindTweenData (tweeny::tween<float>& t)
    {
        for (TweenData& tween : m_Tweens)
        {
            if (tween.TweenMask & TWEEN_F)
            {
                //  Very nasty - address comparison, but it works!
                if (&tween.TweenF == &t)
                {
                    return &tween;
                }
            }
        }

        return nullptr;
    }

    //--------------------------------------------------------------------------------------------------
}
