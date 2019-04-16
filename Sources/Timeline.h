// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __TIMELINE_H__
#define __TIMELINE_H__

#include "Common.h"
#include "TweenManager.h"

namespace aga
{
    class Timeline
    {
    public:
        Timeline (int id, TweenManager* tweenManager);
        virtual ~Timeline ();

        bool Update (float deltaTime);

        //  Run timeline function immediatelly
        Timeline* Once (int duringMS, asIScriptFunction* func);
        
        //  Run timeline function after specified time period
        Timeline* After (int afterMS, asIScriptFunction* func);
        
        //  Used for interpolate between two values in span of the time
        Timeline* During (float from, float to, int duringMS, asIScriptFunction* func);
        Timeline* During (Point from, Point to, int duringMS, asIScriptFunction* func);

        void Clear ();

    private:
        void CleanupFinishedTweens ();
        TweenData* FindTweenData (tweeny::tween<float>& t);
        TweenData* FindTweenData (tweeny::tween<float, float>& t);

    private:
        int m_ID;
        TweenManager* m_TweenManager;

        size_t m_CurrentTweenIndex;
        size_t m_CurrentAfterTweenIndex;
        std::vector<TweenData> m_Tweens;
    };
}

#endif //   __TIMELINE_H__
