// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Script.h"
#include "ScriptManager.h"

namespace aga
{
    //---------------------------------------------------------------------------

    Script::Script (asIScriptModule* module, ScriptManager* manager)
      : m_Module (module)
      , m_Manager (manager)
      , m_UpdateFuncContext (nullptr)
      , m_UpdateFunction (nullptr)
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
        m_UpdateFuncContext = GetContext ("void Update (double deltaTime)");
        m_UpdateFunction = m_UpdateFuncContext->GetFunction ();

        return Lifecycle::Initialize ();
    }

    //--------------------------------------------------------------------------------------------------

    bool Script::Destroy ()
    {
        if (m_UpdateFuncContext != nullptr)
        {
            m_UpdateFuncContext->Release ();
            m_UpdateFuncContext = nullptr;
        }

        return Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    bool Script::Update (double deltaTime)
    {
        m_UpdateFuncContext->Prepare (m_UpdateFunction);
        m_UpdateFuncContext->SetArgDouble (0, deltaTime);

        return InternalRun (m_UpdateFuncContext, false);
    }

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

    bool Script::InternalRun (asIScriptContext* ctx, bool releaseAfterUse)
    {
        int r = ctx->Execute ();

        if (r != asEXECUTION_FINISHED)
        {
            // The execution didn't complete as expected. Determine what happened.
            if (r == asEXECUTION_EXCEPTION)
            {
                // An exception occurred, let the script writer know what happened so it can be corrected.
                printf ("An exception '%s' occurred. Please correct the code and try again.\n", ctx->GetExceptionString ());
            }

            return false;
        }

        if (releaseAfterUse)
        {
            ctx->Release ();
            ctx = nullptr;
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------
}
