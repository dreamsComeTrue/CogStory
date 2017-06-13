// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __LIFECYCLE_H__
#define __LIFECYCLE_H__

namespace aga
{
    class Lifecycle
    {
    public:
        virtual bool Initialize () { m_IsInitialized = true; }
        virtual bool Destroy () { m_IsDestroyed = true; }
        bool IsInitialized () { return m_IsInitialized; }
        bool IsDestroyed () { return m_IsDestroyed; }

    private:
        bool m_IsInitialized;
        bool m_IsDestroyed;
    };
}

#endif //   __LIFECYCLE_H__
