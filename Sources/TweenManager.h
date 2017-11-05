// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __TWEEN_MANAGER_H__
#define __TWEEN_MANAGER_H__

#include "Common.h"

namespace aga
{
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

        void AddTween (int id, float from, float to, int during, asIScriptFunction* func);
        void AddTween (int id, Point from, Point to, int during, asIScriptFunction* func);

    private:
        MainLoop* m_MainLoop;
        std::map<int, tweeny::tween<float>> m_Tweens;
        std::map<int, tweeny::tween<float, float>> m_Tweens2;
        std::map<int, asIScriptFunction*> m_Callbacks;
    };
}

#endif //   __TWEEN_MANAGER_H__
