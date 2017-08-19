// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "ScriptManager.h"
#include "MainLoop.h"
#include "Screen.h"
#include "Script.h"

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

    void print (std::string& msg) { printf ("%s", msg.c_str ()); }

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

        return Lifecycle::Initialize ();
    }

    //--------------------------------------------------------------------------------------------------

    bool ScriptManager::Destroy ()
    {
        for (std::map<std::string, Script*>::iterator it = m_Scripts.begin (); it != m_Scripts.end (); ++it)
        {
            SAFE_DELETE (it->second);
        }

        m_ScriptEngine->ShutDownAndRelease ();

        return Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    Script* ScriptManager::LoadScriptFromFile (const std::string& path, const std::string& moduleName)
    {
        std::ifstream file (path.c_str ());
        std::string text;
        std::stringstream strStream;

        strStream << file.rdbuf (); // read the file
        text = strStream.str ();    // str holds the content of the file
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
        Script* script = new Script (mod, this, moduleName);
        script->Initialize ();

        m_Scripts.insert (std::make_pair (moduleName, script));

        script->Run ("void Start ()");

        return script;
    }

    //--------------------------------------------------------------------------------------------------

    static void ConstructPoint (Point* ptr) { new (ptr) Point (); }

    //--------------------------------------------------------------------------------------------------

    static void ConstructPointCopy (const Point& vector, Point* ptr) { new (ptr) Point (vector); }

    //--------------------------------------------------------------------------------------------------

    static void ConstructPointXY (double x, double y, Point* ptr) { new (ptr) Point (x, y); }

    //--------------------------------------------------------------------------------------------------

    static void Log (Point& point) { printf ("Point [X = %f, Y = %f]\n", point.X, point.Y); }

    //--------------------------------------------------------------------------------------------------

    void ScriptManager::RegisterAPI ()
    {
        //  Point
        m_ScriptEngine->RegisterObjectType ("Point", sizeof (Point), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
        m_ScriptEngine->RegisterObjectProperty ("Point", "double X", asOFFSET (Point, X));
        m_ScriptEngine->RegisterObjectProperty ("Point", "double Y", asOFFSET (Point, Y));
        m_ScriptEngine->RegisterObjectProperty ("Point", "double Width", asOFFSET (Point, Width));
        m_ScriptEngine->RegisterObjectProperty ("Point", "double Height", asOFFSET (Point, Height));

        m_ScriptEngine->RegisterGlobalFunction ("void Log (Point &in)", asFUNCTIONPR (Log, (Point&), void), asCALL_CDECL);

        //  Player
        m_ScriptEngine->RegisterObjectType ("Player", sizeof (Player), asOBJ_VALUE | asOBJ_POD);
        m_ScriptEngine->RegisterGlobalProperty ("Player player", &m_MainLoop->GetSceneManager ()->GetPlayer ());
        m_ScriptEngine->RegisterObjectMethod (
          "Player", "void SetPosition (Point)", asMETHODPR (Player, SetPosition, (const Point&), void), asCALL_THISCALL);
        m_ScriptEngine->RegisterObjectMethod (
          "Player", "void SetPosition (double, double)", asMETHODPR (Player, SetPosition, (double, double), void), asCALL_THISCALL);
        m_ScriptEngine->RegisterObjectMethod ("Player", "void Move (double, double)", asMETHOD (Player, Move), asCALL_THISCALL);
        m_ScriptEngine->RegisterObjectMethod ("Player", "Point GetPosition ()", asMETHOD (Player, GetPosition), asCALL_THISCALL);
        m_ScriptEngine->RegisterObjectMethod ("Player", "Point GetSize ()", asMETHOD (Player, GetSize), asCALL_THISCALL);

        //  Camera
        m_ScriptEngine->RegisterObjectType ("Camera", sizeof (Camera), asOBJ_VALUE | asOBJ_POD);
        m_ScriptEngine->RegisterGlobalProperty ("Camera camera", &m_MainLoop->GetSceneManager ()->GetCamera ());
        m_ScriptEngine->RegisterObjectMethod (
          "Camera", "void SetOffset (float dx, float dy)", asMETHOD (Camera, SetOffset), asCALL_THISCALL);

        //  Global
        m_ScriptEngine->RegisterGlobalFunction ("void Log(const string &in)", asFUNCTION (print), asCALL_CDECL);
        m_ScriptEngine->RegisterGlobalFunction (
          "double GetDeltaTime ()", asMETHOD (Screen, GetDeltaTime), asCALL_THISCALL_ASGLOBAL, m_MainLoop->GetScreen ());
        m_ScriptEngine->RegisterGlobalFunction (
          "Point GetScreenSize ()", asMETHOD (Screen, GetScreenSize), asCALL_THISCALL_ASGLOBAL, m_MainLoop->GetScreen ());

        //  Tweening
        m_ScriptEngine->RegisterFuncdef ("bool TweenFunc (int)");
        m_ScriptEngine->RegisterGlobalFunction ("void AddTween (int, int, int, int, TweenFunc @tf)",
                                                asMETHODPR (TweenManager, AddTween, (int, int, int, int, asIScriptFunction*), void),
                                                asCALL_THISCALL_ASGLOBAL,
                                                m_MainLoop->GetTweenManager ());
    }

    //--------------------------------------------------------------------------------------------------

    asIScriptEngine* ScriptManager::GetEngine () { return m_ScriptEngine; }

    //--------------------------------------------------------------------------------------------------
}
