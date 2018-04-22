// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __SCRIPT_MANAGER_H__
#define __SCRIPT_MANAGER_H__

#include "Common.h"

namespace aga
{
    class MainLoop;
    class Script;

    class ScriptManager : public Lifecycle
    {
    public:
        ScriptManager (MainLoop* mainLoop);
        virtual ~ScriptManager ();
        bool Initialize ();
        bool Destroy ();

        Script* LoadScriptFromFile (const std::string& path, const std::string& moduleName);
        Script* LoadScriptFromText (const std::string& text, const std::string& moduleName);
        asIScriptEngine* GetEngine ();
        asIScriptContext* GetContext ();

        void RemoveScript (const std::string& name);
        void RemoveScript (Script* script);

        Script* GetScriptByModuleName (const std::string& moduleName);

    private:
        void RegisterAPI ();

    private:
        MainLoop* m_MainLoop;
        asIScriptEngine* m_ScriptEngine;
        std::map<std::string, Script*> m_Scripts;
    };

    extern char g_ScriptErrorBuffer[1024];
}

#endif //   __SCRIPT_MANAGER_H__
