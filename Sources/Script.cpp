// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Script.h"
#include "ScriptManager.h"

namespace aga
{
    //---------------------------------------------------------------------------

    Script::Script (asIScriptModule* module, ScriptManager* manager, const std::string& name)
        : m_Module (module)
        , m_Manager (manager)
        , m_FuncContext (nullptr)
        , m_UpdateFunction (nullptr)
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
        m_FuncContext = m_Manager->GetEngine ()->CreateContext ();
        m_FuncContext = GetContext ("void Update (double deltaTime)");
        m_UpdateFunction = m_FuncContext->GetFunction ();

        Run ("void Start ()");

        return Lifecycle::Initialize ();
    }

    //--------------------------------------------------------------------------------------------------

    bool Script::Destroy ()
    {
        if (m_FuncContext != nullptr)
        {
            m_FuncContext = nullptr;
        }

        return Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    bool Script::Update (float deltaTime)
    {
        //      m_FuncContext->Prepare (m_UpdateFunction);
        //    m_FuncContext->SetArgDouble (0, deltaTime);

        //  return InternalRun ();

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool Script::Run (const std::string& functionName)
    {
        GetContext (functionName);
        return InternalRun ();
    }

    //--------------------------------------------------------------------------------------------------

    bool Script::Run (const std::string& functionName, double arg0)
    {
        GetContext (functionName);
        m_FuncContext->SetArgDouble (0, arg0);

        return InternalRun ();
    }

    //--------------------------------------------------------------------------------------------------

    std::string Script::GetName () { return m_Name; }

    //--------------------------------------------------------------------------------------------------

    asIScriptContext* Script::GetContext (const std::string& functionName)
    {
        asIScriptFunction* func = m_Module->GetFunctionByDecl (functionName.c_str ());

        if (!func)
        {
            return nullptr;
        }

        // Create our context, prepare it, and then execute
        m_FuncContext = m_Module->GetEngine ()->RequestContext ();
        m_FuncContext->Prepare (func);

        return m_FuncContext;
    }

    //--------------------------------------------------------------------------------------------------

    bool Script::InternalRun ()
    {
        int r = m_FuncContext->Execute ();

        if (r != asEXECUTION_FINISHED)
        {
            // The execution didn't complete as expected. Determine what happened.
            if (r == asEXECUTION_EXCEPTION)
            {
                // An exception occurred, let the script writer know what happened so it can be corrected.
                printf ("An exception '%s' occurred. Please correct the code and try again.\n",
                    m_FuncContext->GetExceptionString ());
            }

            return false;
        }

        m_FuncContext->GetEngine ()->ReturnContext (m_FuncContext);

        return true;
    }

    //--------------------------------------------------------------------------------------------------
}
