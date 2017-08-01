// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "ScriptManager.h"
#include "MainLoop.h"
#include "Screen.h"
#include "Script.h"

#include <sstream>

namespace aga
{
    //---------------------------------------------------------------------------

    void MessageCallback (const asSMessageInfo* msg, void* param)
    {
        const char* type = "ERR ";

        if (msg->type == asMSGTYPE_WARNING)
        {
            type = "WARN";
        }
        else if (msg->type == asMSGTYPE_INFORMATION)
        {
            type = "INFO";
        }

        char buffer[512];
        sprintf (buffer, "%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, type, msg->message);

        printf (buffer);
    }

    //--------------------------------------------------------------------------------------------------

    ScriptManager::ScriptManager (MainLoop* mainLoop)
        : m_MainLoop (mainLoop)
    {
    }

    //--------------------------------------------------------------------------------------------------

    ScriptManager::~ScriptManager ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    bool ScriptManager::Initialize ()
    {
        m_ScriptEngine = asCreateScriptEngine ();

        // Set the message callback to receive information on errors in human readable form.
        int r = m_ScriptEngine->SetMessageCallback (asFUNCTION (MessageCallback), 0, asCALL_CDECL);
        assert (r >= 0);

        RegisterStdString (m_ScriptEngine);
        RegisterAPI ();

        Lifecycle::Initialize ();
        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool ScriptManager::Destroy ()
    {
        for (std::map<std::string, Script*>::iterator it = m_Scripts.begin (); it != m_Scripts.end (); ++it)
        {
            SAFE_DELETE (it->second);
        }

        m_ScriptEngine->ShutDownAndRelease ();

        Lifecycle::Destroy ();
        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool ScriptManager::Update (double deltaTime)
    {
        for (std::map<std::string, Script*>::iterator it = m_Scripts.begin (); it != m_Scripts.end (); ++it)
        {
            it->second->Update (deltaTime);
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    Script* ScriptManager::LoadScriptFromFile (const std::string& path, const std::string& moduleName)
    {
        std::ifstream file (path.c_str ());
        std::string text;
        std::stringstream strStream;

        strStream << file.rdbuf (); // read the file
        text = strStream.str (); // str holds the content of the file
        file.close ();

        return LoadScriptFromText (text, moduleName);
    }

    //--------------------------------------------------------------------------------------------------

    Script* ScriptManager::LoadScriptFromText (const std::string& text, const std::string& moduleName)
    {
        // The CScriptBuilder helper is an add-on that loads the file,
        // performs a pre-processing pass if necessary, and then tells
        // the engine to build a script module.
        CScriptBuilder builder;

        int r = builder.StartNewModule (m_ScriptEngine, moduleName.c_str ());
        if (r < 0)
        {
            // If the code fails here it is usually because there
            // is no more memory to allocate the module
            printf ("Unrecoverable error while starting a new module.\n");
            return nullptr;
        }

        r = builder.AddSectionFromMemory (moduleName.c_str (), text.c_str ());

        if (r < 0)
        {
            // The builder wasn't able to load the file. Maybe the file
            // has been removed, or the wrong name was given, or some
            // preprocessing commands are incorrectly written.
            printf ("Please correct the errors in the script and try again.\n");
            return nullptr;
        }

        r = builder.BuildModule ();

        if (r < 0)
        {
            // An error occurred. Instruct the script writer to fix the
            // compilation errors that were listed in the output stream.
            printf ("Please correct the errors in the script and try again.\n");
            return nullptr;
        }

        asIScriptModule* mod = m_ScriptEngine->GetModule (moduleName.c_str ());
        Script* script = new Script (mod, this);

        m_Scripts.insert (std::make_pair (moduleName, script));

        script->Run ("void Start ()");

        return script;
    }

    //--------------------------------------------------------------------------------------------------

    static void ConstructPoint (Point* ptr) { new (ptr) Point (); }

    //--------------------------------------------------------------------------------------------------

    static void ConstructPointCopy (const Point& vector, Point* ptr) { new (ptr) Point (vector); }

    //--------------------------------------------------------------------------------------------------

    static void ConstructPointXY (float x, float y, Point* ptr) { new (ptr) Point (x, y); }

    //--------------------------------------------------------------------------------------------------

    static void Log (Point& point) { printf ("Point [X = %f, Y = %f]\n", point.X, point.Y); }

    //--------------------------------------------------------------------------------------------------

    void ScriptManager::RegisterAPI ()
    {
        //  Point
        m_ScriptEngine->RegisterObjectType ("Point", sizeof (Point), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CAK);
        m_ScriptEngine->RegisterObjectBehaviour (
            "Point", asBEHAVE_CONSTRUCT, "void Point()", asFUNCTION (ConstructPoint), asCALL_CDECL_OBJLAST);
        m_ScriptEngine->RegisterObjectBehaviour ("Point", asBEHAVE_CONSTRUCT, "void f(const Point &in)",
            asFUNCTION (ConstructPointCopy), asCALL_CDECL_OBJLAST);
        m_ScriptEngine->RegisterObjectBehaviour (
            "Point", asBEHAVE_CONSTRUCT, "void f(double, double)", asFUNCTION (ConstructPointXY), asCALL_CDECL_OBJLAST);
        m_ScriptEngine->RegisterObjectProperty ("Point", "double X", offsetof (Point, X));
        m_ScriptEngine->RegisterObjectProperty ("Point", "double Y", offsetof (Point, Y));
        m_ScriptEngine->RegisterObjectProperty ("Point", "double Width", offsetof (Point, Width));
        m_ScriptEngine->RegisterObjectProperty ("Point", "double Height", offsetof (Point, Height));

        m_ScriptEngine->RegisterGlobalFunction (
            "void Log (Point &in)", asFUNCTIONPR (Log, (Point&), void), asCALL_CDECL);

        //  Player
        m_ScriptEngine->RegisterObjectType ("Player", sizeof(Player), asOBJ_VALUE | asOBJ_POD);
        m_ScriptEngine->RegisterGlobalProperty ("Player player", &m_MainLoop->GetSceneManager ()->GetPlayer ());
        m_ScriptEngine->RegisterObjectMethod ("Player", "void SetPosition (double X, double Y)",
            asMETHODPR (Player, SetPosition, (double, double), void), asCALL_THISCALL);
        m_ScriptEngine->RegisterObjectMethod (
            "Player", "void Move (double dx, double dy)", asMETHOD (Player, Move), asCALL_THISCALL);

        //  Global
        m_ScriptEngine->RegisterGlobalFunction ("double GetDeltaTime ()", asMETHOD (Screen, GetDeltaTime),
            asCALL_THISCALL_ASGLOBAL, m_MainLoop->GetScreen ());
    }

    //--------------------------------------------------------------------------------------------------

    asIScriptEngine* ScriptManager::GetEngine () { return m_ScriptEngine; }

    //--------------------------------------------------------------------------------------------------
}