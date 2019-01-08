// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Scriptable.h"
#include "Script.h"
#include "ScriptManager.h"

namespace aga
{
	//--------------------------------------------------------------------------------------------------

	Scriptable::Scriptable ()
		: m_ScriptManager (nullptr)
		, m_Enabled (true)
	{
	}

	//--------------------------------------------------------------------------------------------------

	Scriptable::Scriptable (ScriptManager* scriptManager)
		: m_ScriptManager (scriptManager)
		, m_Enabled (true)
	{
	}

	//--------------------------------------------------------------------------------------------------

	Scriptable::Scriptable (const Scriptable& rhs)
	{
		this->m_ScriptManager = rhs.m_ScriptManager;
		this->m_Scripts = rhs.m_Scripts;
		this->m_Enabled = rhs.m_Enabled;
	}

	//--------------------------------------------------------------------------------------------------

	Scriptable::~Scriptable () { RemoveAllScripts (); }

	//--------------------------------------------------------------------------------------------------

	Script* Scriptable::AttachScript (const std::string& name, const std::string& path)
	{
		ScriptMetaData* found = GetScript (name);

		if (!found)
		{
			Script* script = m_ScriptManager->LoadScriptFromFile (path, name);

			ScriptMetaData meta = {name, path, script};
			m_Scripts.push_back (meta);

			return script;
		}
		else
		{
			return found->ScriptObj;
		}
	}

	//--------------------------------------------------------------------------------------------------

	Script* Scriptable::AttachScript (Script* script, const std::string& path)
	{
		if (!script)
		{
			return nullptr;
		}

		ScriptMetaData* found = GetScript (script->GetName ());

		if (!found)
		{
			ScriptMetaData meta = {script->GetName (), path, script};
			m_Scripts.push_back (meta);
		}

		return script;
	}

	//--------------------------------------------------------------------------------------------------

	void Scriptable::RemoveScript (const std::string& name)
	{
		for (std::vector<ScriptMetaData>::iterator it = m_Scripts.begin (); it != m_Scripts.end (); ++it)
		{
			if (it->Name == name)
			{
				m_ScriptManager->RemoveScript (name);
				m_Scripts.erase (it);

				return;
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Scriptable::RemoveScript (Script* script) { RemoveScript (script->GetName ()); }

	//--------------------------------------------------------------------------------------------------

	void Scriptable::RemoveAllScripts ()
	{
		for (size_t i = 0; i < m_Scripts.size (); ++i)
		{
			RemoveScript (m_Scripts[i].ScriptObj);
		}

		m_Scripts.clear ();
	}

	//--------------------------------------------------------------------------------------------------

	void Scriptable::UpdateScripts (float deltaTime)
	{
		if (m_Enabled)
		{
			for (ScriptMetaData& script : m_Scripts)
			{
				if (script.ScriptObj)
				{
					script.ScriptObj->Update (deltaTime);
				}
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Scriptable::RunAllScripts (const std::string& functionName)
	{
		if (m_Enabled)
		{
			for (ScriptMetaData& script : m_Scripts)
			{
				if (script.ScriptObj)
				{
					script.ScriptObj->Run (functionName);
				}
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Scriptable::RunAllScripts (const std::string& functionName, void* obj)
	{
		if (m_Enabled)
		{
			for (ScriptMetaData& script : m_Scripts)
			{
				if (script.ScriptObj)
				{
					script.ScriptObj->Run (functionName, obj);
				}
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	ScriptMetaData* Scriptable::GetScript (const std::string& name)
	{
		for (ScriptMetaData& sc : m_Scripts)
		{
			if (sc.Name == name)
			{
				return &sc;
			}
		}

		return nullptr;
	}

	//--------------------------------------------------------------------------------------------------

	void Scriptable::EnableScripts () { m_Enabled = true; }

	//--------------------------------------------------------------------------------------------------

	void Scriptable::DisableScripts () { m_Enabled = false; }

	//--------------------------------------------------------------------------------------------------

#ifdef _MSC_VER
	std::optional<ScriptMetaData> Scriptable::GetScriptByName (const std::string& name)
#else
	std::experimental::optional<ScriptMetaData> Scriptable::GetScriptByName (const std::string& name)
#endif
	{
		for (std::vector<ScriptMetaData>::iterator it = m_Scripts.begin (); it != m_Scripts.end (); ++it)
		{
			if (it->Name == name)
			{
#ifdef _MSC_VER
				return std::make_optional (*it);
#else
				return std::experimental::make_optional (*it);
#endif
			}
		}

#ifdef _MSC_VER
		return std::optional<ScriptMetaData> ();
#else
		return std::experimental::optional<ScriptMetaData> ();
#endif
	}

	//--------------------------------------------------------------------------------------------------

	void Scriptable::ReloadScript (const std::string& name)
	{
#ifdef _MSC_VER
		std::optional<ScriptMetaData> metaScript = GetScriptByName (name);
#else
		std::experimental::optional<ScriptMetaData> metaScript = GetScriptByName (name);
#endif

		if (metaScript)
		{
			std::string path = metaScript.value ().Path;

			RemoveScript (name);
			AttachScript (name, path);

			Log ("Script: %s (%s) reloaded.\n", name.c_str (), path.c_str ());
		}
	}

	//--------------------------------------------------------------------------------------------------
}
