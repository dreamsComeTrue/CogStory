// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __TWEEN_MANAGER_H__
#define __TWEEN_MANAGER_H__

#include "Common.h"

namespace aga
{
    struct TweenData
    {
        int ID = -1;
        tweeny::tween<float, float> Tween;
        asIScriptFunction* CallbackFunc = nullptr;
        asIScriptFunction* FinishFunc = nullptr;
    };

    class MainLoop;

    class TweenManager : public Lifecycle
    {
    public:
        TweenManager (MainLoop* mainLoop);
        virtual ~TweenManager ();
        bool Initialize ();
        bool Destroy ();

        void AddTween (int id, float from, float to, int during, std::function<bool(float)>);
        bool Update (float deltaTime);
        MainLoop* GetMainLoop ();

        // void AddTween (int id, float from, float to, int during, asIScriptFunction* func);
        void AddTween (
            int id, Point from, Point to, int during, asIScriptFunction* func, asIScriptFunction* finishFunc);

    private:
        asIScriptFunction* FindCallback (tweeny::tween<float, float>& t);

    private:
        MainLoop* m_MainLoop;
        std::vector<TweenData> m_Tweens;
    };
}

#endif //   __TWEEN_MANAGER_H__
