// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __LIFECYCLE_H__
#define __LIFECYCLE_H__

namespace aga
{
    class Lifecycle
    {
    public:
        virtual ~Lifecycle () {}

        virtual bool Initialize ()
        {
            m_IsDestroyed = false;
            m_IsInitialized = true;

            return true;
        }

        virtual bool Destroy ()
        {
            m_IsDestroyed = true;
            m_IsInitialized = false;

            return true;
        }

        bool IsInitialized () { return m_IsInitialized; }
        bool IsDestroyed () { return m_IsDestroyed; }

    protected:
        bool m_IsInitialized;
        bool m_IsDestroyed;
    };
}

#endif //   __LIFECYCLE_H__
