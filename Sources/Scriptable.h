// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __SCRIPTABLE_H__
#define __SCRIPTABLE_H__

#include "Common.h"

namespace aga
{
    class ScriptManager;
    class Script;

    struct ScriptMetaData
    {
        std::string Name;
        std::string Path;
        Script* ScriptObj = nullptr;

        bool const operator< (const ScriptMetaData& other) const { return Name < other.Name; }
    };

    class Scriptable
    {
    public:
        Scriptable (ScriptManager* scriptManager);
        virtual ~Scriptable ();

        void AttachScript (const std::string& name, const std::string& path);
        void AttachScript (Script* script, const std::string& path);

        void RemoveScript (const std::string& name);
        void RemoveScript (Script* script);
        void RemoveAllScripts ();

        void UpdateScripts (float deltaTime);
        void RunAllScripts (const std::string& functionName);
        void RunAllScripts (const std::string& functionName, void* obj);

        ScriptMetaData* GetScript (const std::string& name);

#ifdef _MSC_VER
        std::optional<ScriptMetaData> GetScriptByName (const std::string& name);
#else
        std::experimental::optional<ScriptMetaData> GetScriptByName (const std::string& name);
#endif

        void ReloadScript (const std::string& name);

        std::vector<ScriptMetaData>& GetScripts () { return m_Scripts; }

    protected:
        ScriptManager* m_ScriptManager;
        std::vector<ScriptMetaData> m_Scripts;
    };
}

#endif //   __SCRIPTABLE_H__
