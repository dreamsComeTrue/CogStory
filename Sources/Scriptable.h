// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __SCRIPTABLE_H__
#define __SCRIPTABLE_H__

#include "Common.h"
#include "Script.h"

namespace aga
{
    struct ScriptMetaData
    {
        std::string Name;
        std::string Path;

        bool const operator< (const ScriptMetaData& other) const { return Name < other.Name; }
    };

    typedef std::map<ScriptMetaData, Script*>::iterator ScripIterator;

    class Scriptable
    {
    public:
        void AttachScript (Script* script, const std::string& path)
        {
            bool found = false;

            for (ScripIterator it = m_Scripts.begin (); it != m_Scripts.end (); ++it)
            {
                if (it->first.Name == script->GetName ())
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                ScriptMetaData meta = { script->GetName (), path };
                m_Scripts.insert (std::make_pair (meta, script));
            }
        }

        void RemoveScript (Script* script)
        {
            bool found = false;

            ScripIterator it = m_Scripts.begin ();
            for (; it != m_Scripts.end (); ++it)
            {
                if (it->first.Name == script->GetName ())
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                m_Scripts.erase (it);
            }
        }

        void UpdateScripts (float deltaTime)
        {
            for (std::map<ScriptMetaData, Script*>::iterator it = m_Scripts.begin (); it != m_Scripts.end (); ++it)
            {
                it->second->Update (deltaTime);
            }
        }

        void RunAllScripts (const std::string& functionName)
        {
            for (std::map<ScriptMetaData, Script*>::iterator it = m_Scripts.begin (); it != m_Scripts.end (); ++it)
            {
                it->second->Run (functionName);
            }
        }

    protected:
        std::map<ScriptMetaData, Script*> m_Scripts;
    };
}

#endif //   __SCRIPTABLE_H__
