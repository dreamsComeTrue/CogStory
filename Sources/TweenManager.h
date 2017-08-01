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

        void AddTween (int id, int from, int to, int during, std::function<bool(int)>);
        bool Update (double deltaTime);
        MainLoop* GetMainLoop ();

    private:
        MainLoop* m_MainLoop;
        std::map<int, tweeny::tween<int>> m_Tweens;
    };
}

#endif //   __TWEEN_MANAGER_H__
