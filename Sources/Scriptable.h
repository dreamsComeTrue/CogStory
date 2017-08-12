// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __SCRIPTABLE_H__
#define __SCRIPTABLE_H__

#include "Common.h"
#include "Script.h"

namespace aga
{
    class Scriptable
    {
    public:
        void AttachScript (Script* script)
        {
            if (m_Scripts.find (script->GetName ()) == m_Scripts.end ())
            {
                m_Scripts.insert (std::make_pair (script->GetName (), script));
            }
        }

        void RemoveScript (Script* script)
        {
            if (m_Scripts.find (script->GetName ()) == m_Scripts.end ())
            {
                m_Scripts.erase (script->GetName ());
            }
        }

        void UpdateScripts (double deltaTime)
        {
            for (std::map<std::string, Script*>::iterator it = m_Scripts.begin (); it != m_Scripts.end (); ++it)
            {
                it->second->Update (deltaTime);
            }
        }

        void RunAllScripts (const std::string& functionName)
        {
            for (std::map<std::string, Script*>::iterator it = m_Scripts.begin (); it != m_Scripts.end (); ++it)
            {
                it->second->Run (functionName);
            }
        }

    protected:
        std::map<std::string, Script*> m_Scripts;
    };
}

#endif //   __SCRIPTABLE_H__
