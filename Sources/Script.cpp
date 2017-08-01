// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Script.h"
#include "ScriptManager.h"

namespace aga
{
    //---------------------------------------------------------------------------

    Script::Script (asIScriptModule* module, ScriptManager* manager)
        : m_Module (module)
        , m_Manager (manager)
    {
    }

    //--------------------------------------------------------------------------------------------------

    Script::~Script ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    bool Script::Initialize ()
    {
        Lifecycle::Initialize ();
        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool Script::Destroy ()
    {
        Lifecycle::Destroy ();
        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool Script::Update (double deltaTime) { return Run ("void Update (double deltaTime)", deltaTime); }

    //--------------------------------------------------------------------------------------------------

    bool Script::Run (const std::string& functionName)
    {
        asIScriptContext* ctx = GetContext (functionName);
        return InternalRun (ctx);
    }

    //--------------------------------------------------------------------------------------------------

    bool Script::Run (const std::string& functionName, double arg0)
    {
        asIScriptContext* ctx = GetContext (functionName);
        ctx->SetArgDouble (0, arg0);

        return InternalRun (ctx);
    }

    //--------------------------------------------------------------------------------------------------

    asIScriptContext* Script::GetContext (const std::string& functionName)
    {
        asIScriptFunction* func = m_Module->GetFunctionByDecl (functionName.c_str ());

        if (func == 0)
        {
            return nullptr;
        }

        // Create our context, prepare it, and then execute
        asIScriptContext* ctx = m_Manager->GetEngine ()->CreateContext ();
        ctx->Prepare (func);

        return ctx;
    }

    //--------------------------------------------------------------------------------------------------

    bool Script::InternalRun (asIScriptContext* ctx)
    {
        int r = ctx->Execute ();

        if (r != asEXECUTION_FINISHED)
        {
            // The execution didn't complete as expected. Determine what happened.
            if (r == asEXECUTION_EXCEPTION)
            {
                // An exception occurred, let the script writer know what happened so it can be corrected.
                printf (
                    "An exception '%s' occurred. Please correct the code and try again.\n", ctx->GetExceptionString ());
            }

            return false;
        }

        ctx->Release ();

        return true;
    }

    //--------------------------------------------------------------------------------------------------
}
