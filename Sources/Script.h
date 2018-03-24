// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __SCRIPT_H__
#define __SCRIPT_H__

#include "Common.h"

namespace aga
{
    class ScriptManager;

    class Script : public Lifecycle
    {
    public:
        Script (asIScriptModule* module, ScriptManager* manager, const std::string& name);
        virtual ~Script ();
        bool Initialize ();
        bool Destroy ();

        bool Update (float deltaTime);

        bool Run (const std::string& functionName);
        bool Run (const std::string& functionName, float arg0);
        bool Run (const std::string& functionName, int arg0);
        bool Run (const std::string& functionName, void* arg0);

        std::string GetName ();
        asIScriptContext* GetContext ();

    private:
        asIScriptContext* GetContext (const std::string& functionName);
        bool InternalRun (asIScriptContext* ctx);

    private:
        std::string m_Name;
        ScriptManager* m_Manager;
        asIScriptModule* m_Module;
    };
}

#endif //   __SCRIPT_H__
