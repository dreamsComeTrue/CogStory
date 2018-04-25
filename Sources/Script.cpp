// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Script.h"
#include "ScriptManager.h"

namespace aga
{
    //---------------------------------------------------------------------------

    Script::Script (asIScriptModule* module, ScriptManager* manager, const std::string& name)
        : m_Module (module)
        , m_Manager (manager)
        , m_Name (name)
    {
    }

    //--------------------------------------------------------------------------------------------------

    Script::~Script ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool Script::Initialize ()
    {
        Lifecycle::Initialize ();

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool Script::Destroy () { return Lifecycle::Destroy (); }

    //--------------------------------------------------------------------------------------------------

    bool Script::Update (float deltaTime)
    {
        asIScriptContext* ctx = GetContext ("void Update (float deltaTime)");

        if (ctx)
        {
            ctx->SetArgFloat (0, deltaTime);

            return InternalRun (ctx);
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    bool Script::Run (const std::string& functionName)
    {
        asIScriptContext* ctx = GetContext (functionName);

        if (ctx)
        {
            return InternalRun (ctx);
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    bool Script::Run (const std::string& functionName, float arg0)
    {
        asIScriptContext* ctx = GetContext (functionName);

        if (ctx)
        {
            ctx->SetArgFloat (0, arg0);

            return InternalRun (ctx);
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    bool Script::Run (const std::string& functionName, int arg0)
    {
        asIScriptContext* ctx = GetContext (functionName);

        if (ctx)
        {
            ctx->SetArgDWord (0, arg0);

            return InternalRun (ctx);
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    bool Script::Run (const std::string& functionName, void* arg0)
    {
        asIScriptContext* ctx = GetContext (functionName);

        if (ctx)
        {
            ctx->SetArgObject (0, arg0);

            return InternalRun (ctx);
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    asIScriptContext* Script::GetContext (const std::string& functionName)
    {
        asIScriptFunction* func = m_Module->GetFunctionByDecl (functionName.c_str ());

        if (!func)
        {
            return nullptr;
        }

        asIScriptContext* ctx = m_Manager->GetContext ();

        // Create our context, prepare it, and then execute
        ctx->Prepare (func);

        return ctx;
    }

    //--------------------------------------------------------------------------------------------------

    bool Script::InternalRun (asIScriptContext* ctx)
    {
        if (!ctx)
        {
            return false;
        }

        int r = ctx->Execute ();

        if (r != asEXECUTION_FINISHED)
        {
            // The execution didn't complete as expected. Determine what happened.
            if (r == asEXECUTION_EXCEPTION)
            {
                // An exception occurred, let the script writer know what happened so it can be corrected.
                Log ("An exception '%s' occurred. Please correct the code and try again.\n",
                     ctx->GetExceptionString ());
            }

            return false;
        }

        ctx->Unprepare ();
        ctx->GetEngine ()->ReturnContext (ctx);

        return true;
    }

    //--------------------------------------------------------------------------------------------------
}
