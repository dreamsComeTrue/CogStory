// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "ScriptManager.h"
#include "MainLoop.h"
#include "Scene.h"
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
    }

    //--------------------------------------------------------------------------------------------------

    bool ScriptManager::Initialize ()
    {
        Lifecycle::Initialize ();

        m_ScriptEngine = asCreateScriptEngine ();

        // Set the message callback to receive information on errors in human readable form.
        int r = m_ScriptEngine->SetMessageCallback (asFUNCTION (MessageCallback), 0, asCALL_CDECL);
        assert (r >= 0);

        RegisterStdString (m_ScriptEngine);
        RegisterAPI ();

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

        return Lifecycle::Destroy ();
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

        Script* script = new Script (builder.GetModule (), this, moduleName);
        m_Scripts.insert (std::make_pair (moduleName, script));

        script->Initialize ();

        return script;
    }

    //--------------------------------------------------------------------------------------------------

    static void ConstructPoint (Point* ptr) { new (ptr) Point (); }

    //--------------------------------------------------------------------------------------------------

    static void ConstructPointCopy (const Point& vector, Point* ptr) { new (ptr) Point (vector); }

    //--------------------------------------------------------------------------------------------------

    static void ConstructPointXY (double x, double y, Point* ptr) { new (ptr) Point (x, y); }

    //--------------------------------------------------------------------------------------------------

    static void Log (const std::string& data) { printf ("%s\n", data); }

    //--------------------------------------------------------------------------------------------------

    static void Log (Point& point) { printf ("Point [X = %f, Y = %f]\n", point.X, point.Y); }

    //--------------------------------------------------------------------------------------------------

    static void Log (double data) { printf ("%f\n", data); }

    //--------------------------------------------------------------------------------------------------

    void ScriptManager::RegisterAPI ()
    {
        //  Point
        int r = m_ScriptEngine->RegisterObjectType (
            "Point", sizeof (Point), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CAK);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectBehaviour (
            "Point", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION (ConstructPoint), asCALL_CDECL_OBJLAST);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectBehaviour ("Point", asBEHAVE_CONSTRUCT, "void f(const Point &in)",
            asFUNCTION (ConstructPointCopy), asCALL_CDECL_OBJLAST);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectBehaviour (
            "Point", asBEHAVE_CONSTRUCT, "void f(double, double)", asFUNCTION (ConstructPointXY), asCALL_CDECL_OBJLAST);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectProperty ("Point", "double X", asOFFSET (Point, X));
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectProperty ("Point", "double Y", asOFFSET (Point, Y));
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectProperty ("Point", "double Width", asOFFSET (Point, Width));
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectProperty ("Point", "double Height", asOFFSET (Point, Height));
        assert (r >= 0);

        r = m_ScriptEngine->RegisterGlobalFunction (
            "void Log (Point &in)", asFUNCTIONPR (Log, (Point&), void), asCALL_CDECL);
        assert (r >= 0);

        //  Scene
        r = m_ScriptEngine->RegisterObjectType ("SceneManager", sizeof (SceneManager), asOBJ_VALUE | asOBJ_POD);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalProperty ("SceneManager sceneManager", &m_MainLoop->GetSceneManager ());
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("SceneManager", "Point GetSpawnPoint (const string &in)",
            asMETHOD (SceneManager, GetSpawnPoint), asCALL_THISCALL);

        //   r = m_ScriptEngine->RegisterGlobalFunction ("Point GetSpawnPoint (const string &in)",
        //     asMETHOD (SceneManager, GetSpawnPoint), asCALL_THISCALL_ASGLOBAL, &m_MainLoop->GetSceneManager ());
        assert (r >= 0);

        //  Player
        r = m_ScriptEngine->RegisterObjectType ("Player", sizeof (Player), asOBJ_VALUE | asOBJ_POD);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalProperty ("Player player", &m_MainLoop->GetSceneManager ().GetPlayer ());
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("Player", "void SetPosition (const Point &in)",
            asMETHODPR (Player, SetPosition, (const Point&), void), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("Player", "void SetPosition (double, double)",
            asMETHODPR (Player, SetPosition, (double, double), void), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "Player", "Point GetPosition ()", asMETHOD (Player, GetPosition), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "Player", "Point GetSize ()", asMETHOD (Player, GetSize), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "Player", "void Move (double, double)", asMETHOD (Player, Move), asCALL_THISCALL);
        assert (r >= 0);

        //  Camera
        r = m_ScriptEngine->RegisterObjectType ("Camera", sizeof (Camera), asOBJ_VALUE | asOBJ_POD);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalProperty ("Camera camera", &m_MainLoop->GetSceneManager ().GetCamera ());
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "Camera", "void SetTranslate (float dx, float dy)", asMETHOD (Camera, SetTranslate), asCALL_THISCALL);
        assert (r >= 0);

        //  Screen
        r = m_ScriptEngine->RegisterObjectType ("Screen", sizeof (Screen), asOBJ_VALUE | asOBJ_POD);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalProperty ("Screen screen", m_MainLoop->GetScreen ());
        assert (r >= 0);

        //  Global
        r = m_ScriptEngine->RegisterGlobalFunction (
            "void Log(const string &in)", asFUNCTIONPR (Log, (const std::string&), void), asCALL_CDECL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction (
            "void Log (double)", asFUNCTIONPR (Log, (double), void), asCALL_CDECL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction ("const Point& GetWindowSize ()", asMETHOD (Screen, GetWindowSize),
            asCALL_THISCALL_ASGLOBAL, m_MainLoop->GetScreen ());
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction ("double GetDeltaTime ()", asMETHOD (Screen, GetDeltaTime),
            asCALL_THISCALL_ASGLOBAL, m_MainLoop->GetScreen ());
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction (
            "double GetFPS ()", asMETHOD (Screen, GetFPS), asCALL_THISCALL_ASGLOBAL, m_MainLoop->GetScreen ());
        assert (r >= 0);

        //  Tweening
        r = m_ScriptEngine->RegisterFuncdef ("bool TweenFunc (int)");
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction ("void AddTween (int, int, int, int, TweenFunc @tf)",
            asMETHODPR (TweenManager, AddTween, (int, int, int, int, asIScriptFunction*), void),
            asCALL_THISCALL_ASGLOBAL, &m_MainLoop->GetTweenManager ());
        assert (r >= 0);
    }

    //--------------------------------------------------------------------------------------------------

    asIScriptEngine* ScriptManager::GetEngine () { return m_ScriptEngine; }

    //--------------------------------------------------------------------------------------------------
}
