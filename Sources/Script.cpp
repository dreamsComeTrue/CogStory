// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Script.h"
#include "ScriptManager.h"

namespace aga
{
    //---------------------------------------------------------------------------

    std::string GetCallStack (asIScriptContext* context)
    {
        std::string str ("AngelScript callstack:\n");

        // Append the call stack
        for (asUINT i = 0; i < context->GetCallstackSize (); i++)
        {
            const char* scriptSection;
            int column;
            asIScriptFunction* func = context->GetFunction (i);
            int line = context->GetLineNumber (i, &column, &scriptSection);

            char buffer[1024] = {};
            sprintf (buffer, "\t%s:%s:%d,%d\n", scriptSection, func->GetDeclaration (), line, column);
            str += buffer;
        }

        return str;
    }

    //---------------------------------------------------------------------------

    void ExceptionCallback (asIScriptContext* context)
    {
        char buffer[1024] = {};
        sprintf (buffer, "- Exception '%s' in '%s'\n%s", context->GetExceptionString (),
                 context->GetExceptionFunction ()->GetDeclaration (), GetCallStack (context).c_str ());

        printf ("%s", buffer);
    }

    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------

    Script::Script (asIScriptModule* module, ScriptManager* manager, const std::string& name)
        : m_Module (module)
        , m_Manager (manager)
        , m_FuncContext (nullptr)
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

        m_FuncContext = m_Module->GetEngine ()->RequestContext ();
        m_FuncContext->SetExceptionCallback (asFUNCTION (ExceptionCallback), this, asCALL_THISCALL);

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool Script::Destroy ()
    {
        if (m_FuncContext != nullptr)
        {
            m_FuncContext->GetEngine ()->ReturnContext (m_FuncContext);
            m_FuncContext = nullptr;
        }

        return Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    bool Script::Update (float deltaTime)
    {
        asIScriptContext* fun = GetContext ("void Update (float deltaTime)");

        if (fun)
        {
            m_FuncContext->SetArgFloat (0, deltaTime);

            return InternalRun ();
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    bool Script::Run (const std::string& functionName)
    {
        asIScriptContext* fun = GetContext (functionName);

        if (fun)
        {
            return InternalRun ();
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    bool Script::Run (const std::string& functionName, float arg0)
    {
        asIScriptContext* fun = GetContext (functionName);

        if (fun)
        {
            m_FuncContext->SetArgFloat (0, arg0);

            return InternalRun ();
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    std::string Script::GetName () { return m_Name; }

    //--------------------------------------------------------------------------------------------------

    asIScriptContext* Script::GetContext () { return m_FuncContext; }

    //--------------------------------------------------------------------------------------------------

    asIScriptContext* Script::GetContext (const std::string& functionName)
    {
        asIScriptFunction* func = m_Module->GetFunctionByDecl (functionName.c_str ());

        if (!func)
        {
            return nullptr;
        }

        // Create our context, prepare it, and then execute
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

        m_FuncContext->Unprepare ();

        return true;
    }

    //--------------------------------------------------------------------------------------------------
}
