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

        bool Update (double deltaTime);

        Script* LoadScriptFromFile (const std::string& path, const std::string& moduleName);
        Script* LoadScriptFromText (const std::string& text, const std::string& moduleName);
        asIScriptEngine* GetEngine ();

    private:
        void RegisterAPI ();

    private:
        MainLoop* m_MainLoop;
        asIScriptEngine* m_ScriptEngine;
        std::map<std::string, Script*> m_Scripts;
    };
}

#endif //   __SCRIPT_MANAGER_H__
