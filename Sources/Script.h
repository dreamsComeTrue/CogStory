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

        bool Update (double deltaTime);

        bool Run (const std::string& functionName);
        bool Run (const std::string& functionName, double arg0);

        std::string GetName ();

    private:
        asIScriptContext* GetContext (const std::string& functionName);
        bool InternalRun (asIScriptContext* ctx, bool releaseAfterUse = true);

    private:
        std::string m_Name;
        ScriptManager* m_Manager;
        asIScriptModule* m_Module;

        asIScriptContext* m_UpdateFuncContext;
        asIScriptFunction* m_UpdateFunction;
    };
}

#endif //   __SCRIPT_H__
