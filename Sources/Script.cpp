// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Script.h"
#include "ScriptManager.h"

namespace aga
{
	//---------------------------------------------------------------------------

	Script::Script (asIScriptModule* module, ScriptManager* manager, const std::string& name, const std::string& path)
		: m_Manager (manager)
		, m_Name (name)
		, m_Module (module)
		, m_Path (path)
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

	bool Script::Initialize () { return Lifecycle::Initialize (); }

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
			ctx->SetArgDWord (0, static_cast<asDWORD> (arg0));

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
				Log (std::string (
						 std::string (
							 "An exception '%s' occurred. Please correct the code and try again.\nScript name: ")
						 + m_Name + "\n")
						 .c_str (),
					ctx->GetExceptionString ());
			}

			return false;
		}

		ctx->Unprepare ();
		ctx->GetEngine ()->ReturnContext (ctx);

		return true;
	}

	//--------------------------------------------------------------------------------------------------

	void Script::Reload ()
	{
		std::ifstream file (m_Path.c_str ());
		std::stringstream strStream;

		strStream << file.rdbuf (); // read the file
		std::string text = strStream.str (); // str holds the content of the file
		file.close ();

		memset (g_ScriptErrorBuffer, 0, sizeof (g_ScriptErrorBuffer));

		// The CScriptBuilder helper is an add-on that loads the file,
		// performs a pre-processing pass if necessary, and then tells
		// the engine to build a script module.
		CScriptBuilder builder;

		m_Manager->GetEngine ()->DiscardModule (m_Module->GetName ());

		int r = builder.StartNewModule (m_Manager->GetEngine (), m_Name.c_str ());
		if (r < 0)
		{
			// If the code fails here it is usually because there
			// is no more memory to allocate the module
			Log (8000.f, COLOR_ORANGE, "Unrecoverable error while starting a new module.\n");
			return;
		}

		r = builder.AddSectionFromMemory (m_Name.c_str (), text.c_str ());

		if (r < 0)
		{
			// The builder wasn't able to load the file. Maybe the file
			// has been removed, or the wrong name was given, or some
			// preprocessing commands are incorrectly written.
			Log (8000.f, COLOR_ORANGE, "Please correct the errors in the script and try again.\n");
			return;
		}

		r = builder.BuildModule ();

		if (r < 0)
		{
			// An error occurred. Instruct the script writer to fix the
			// compilation errors that were listed in the output stream.
			Log (8000.f, COLOR_ORANGE, "Please correct the errors in the script and try again.\n");
			return;
		}

		m_Module = builder.GetModule ();
	}

	//--------------------------------------------------------------------------------------------------
}
