// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __TWEEN_MANAGER_H__
#define __TWEEN_MANAGER_H__

#include "Common.h"

namespace aga
{
#define TWEEN_F 0x01
#define TWEEN_FF 0x02

    struct TweenData
    {
        int ID = -1;
        tweeny::tween<float> TweenF;
        tweeny::tween<float, float> TweenFF;
        asIScriptFunction* CallbackFunc = nullptr;
        asIScriptFunction* FinishFunc = nullptr;

        char TweenMask = 0;
        bool IsPaused = false;
    };

    class MainLoop;

    class TweenManager : public Lifecycle
    {
    public:
        TweenManager (MainLoop* mainLoop);
        virtual ~TweenManager ();
        bool Initialize ();
        bool Destroy ();

        TweenData& AddTween (int id, float from, float to, int during, std::function<bool(float)>);
        TweenData& AddTween (int id, tweeny::tween<float>& func);
        TweenData& AddTween (int id, Point from, Point to, int during, std::function<bool(float, float)> func);
        TweenData& AddTween (int id, Point from, Point to, int during, std::function<bool(float, float)> func,
                             asIScriptFunction* finishFunc);

        bool Update (float deltaTime);
        MainLoop* GetMainLoop ();

        // void AddTween (int id, float from, float to, int during, asIScriptFunction* func);
        void AddTween (int id, Point from, Point to, int during, asIScriptFunction* func,
                       asIScriptFunction* finishFunc);
        TweenData* GetTween (int id);

        void PauseTween (int id);
        void ResumeTween (int id);

        void Clear ();

    private:
        void CleanupFinishedTweens ();
        TweenData* FindTweenData (tweeny::tween<float, float>& t);

    private:
        MainLoop* m_MainLoop;
        std::vector<TweenData> m_Tweens;
    };
}

#endif //   __TWEEN_MANAGER_H__
