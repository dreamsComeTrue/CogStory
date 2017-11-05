// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __SCRIPTABLE_H__
#define __SCRIPTABLE_H__

#include "Common.h"
#include "Script.h"
#include "ScriptManager.h"

namespace aga
{
    struct ScriptMetaData
    {
        std::string Name;
        std::string Path;
        Script* ScriptObj;

        bool const operator< (const ScriptMetaData& other) const { return Name < other.Name; }
    };

    class Scriptable
    {
    public:
        Scriptable (ScriptManager* scriptManager)
            : m_ScriptManager (scriptManager)
        {
        }

        void AttachScript (Script* script, const std::string& path)
        {
            bool found = false;

            for (std::vector<ScriptMetaData>::iterator it = m_Scripts.begin (); it != m_Scripts.end (); ++it)
            {
                if (it->Name == script->GetName ())
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                ScriptMetaData meta = { script->GetName (), path, script };
                m_Scripts.push_back (meta);
            }
        }

        void RemoveScript (Script* script)
        {
            bool found = false;

            std::vector<ScriptMetaData>::iterator it = m_Scripts.begin ();
            for (; it != m_Scripts.end (); ++it)
            {
                if (it->Name == script->GetName ())
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
            for (std::vector<ScriptMetaData>::iterator it = m_Scripts.begin (); it != m_Scripts.end (); ++it)
            {
                it->ScriptObj->Update (deltaTime);
            }
        }

        void RunAllScripts (const std::string& functionName)
        {
            for (std::vector<ScriptMetaData>::iterator it = m_Scripts.begin (); it != m_Scripts.end (); ++it)
            {
                it->ScriptObj->Run (functionName);
            }
        }

        boost::optional<ScriptMetaData&> GetScriptByName (const std::string& name)
        {
            for (std::vector<ScriptMetaData>::iterator it = m_Scripts.begin (); it != m_Scripts.end (); ++it)
            {
                if (it->Name == name)
                {
                    return *it;
                }
            }

            return boost::optional<ScriptMetaData&> ();
        }

        void ReloadScript (const std::string& name)
        {
            boost::optional<ScriptMetaData&> metaScript = GetScriptByName (name);

            if (metaScript.is_initialized ())
            {
                std::string path = metaScript.get ().Path;
                RemoveScript (metaScript.get ().ScriptObj);

                Script* s = m_ScriptManager->LoadScriptFromFile (GetDataPath () + "scripts/" + path, name);

                AttachScript (s, path);
            }
        }

        std::vector<ScriptMetaData>& GetScripts () { return m_Scripts; }

    protected:
        ScriptManager* m_ScriptManager;
        std::vector<ScriptMetaData> m_Scripts;
    };
}

#endif //   __SCRIPTABLE_H__
