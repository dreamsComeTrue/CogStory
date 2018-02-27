// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "ScriptManager.h"
#include "MainLoop.h"
#include "Scene.h"
#include "Screen.h"
#include "Script.h"
#include "SpeechFrame.h"
#include "SpeechFrameManager.h"

namespace aga
{
    //---------------------------------------------------------------------------

    char g_ScriptErrorBuffer[1024];

    void MessageCallback (const asSMessageInfo* msg, void* param)
    {
        const char* type = "ERROR";

        if (msg->type == asMSGTYPE_WARNING)
        {
            type = "WARN";
        }
        else if (msg->type == asMSGTYPE_INFORMATION)
        {
            type = "INFO";
        }

        sprintf (g_ScriptErrorBuffer, "%s [%d, %d]: %s: %s\n", msg->section, msg->row, msg->col, type, msg->message);

        printf ("%s", g_ScriptErrorBuffer);
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
        memset (g_ScriptErrorBuffer, 0, sizeof (g_ScriptErrorBuffer));

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

    static void ConstructPointXY (float x, float y, Point* ptr) { new (ptr) Point (x, y); }

    //--------------------------------------------------------------------------------------------------

    static void ConstructRect (Rect* ptr) { new (ptr) Rect (); }

    //--------------------------------------------------------------------------------------------------

    static void ConstructRectCopy (const Rect& vector, Rect* ptr) { new (ptr) Rect (vector); }

    //--------------------------------------------------------------------------------------------------

    static void ConstructRectXY (Point topLeft, Point bottomRight, Rect* ptr) { new (ptr) Rect (topLeft, bottomRight); }

    //--------------------------------------------------------------------------------------------------

    static void Log (const std::string& data) { printf ("%s\n", data.c_str ()); }

    //--------------------------------------------------------------------------------------------------

    static void Log (Point& point) { printf ("Point [X = %f, Y = %f]\n", point.X, point.Y); }

    //--------------------------------------------------------------------------------------------------

    static void Log (float data) { printf ("%f\n", data); }

    //--------------------------------------------------------------------------------------------------

    ALLEGRO_COLOR COLOR_BLACK_REF = COLOR_BLACK;

    void ScriptManager::RegisterAPI ()
    {
        //  Point
        int r = m_ScriptEngine->RegisterObjectType (
            "Point", sizeof (Point), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CA | asOBJ_APP_CLASS_ALLFLOATS);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectBehaviour ("Point", asBEHAVE_CONSTRUCT, "void f()",
                                                     asFUNCTION (ConstructPoint), asCALL_CDECL_OBJLAST);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectBehaviour ("Point", asBEHAVE_CONSTRUCT, "void f(const Point &in)",
                                                     asFUNCTION (ConstructPointCopy), asCALL_CDECL_OBJLAST);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectBehaviour ("Point", asBEHAVE_CONSTRUCT, "void f(float, float)",
                                                     asFUNCTION (ConstructPointXY), asCALL_CDECL_OBJLAST);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectProperty ("Point", "float X", asOFFSET (Point, X));
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectProperty ("Point", "float Y", asOFFSET (Point, Y));
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectProperty ("Point", "float Width", asOFFSET (Point, Width));
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectProperty ("Point", "float Height", asOFFSET (Point, Height));
        assert (r >= 0);

        r = m_ScriptEngine->RegisterGlobalFunction ("void Log (Point &in)", asFUNCTIONPR (Log, (Point&), void),
                                                    asCALL_CDECL);
        assert (r >= 0);

        //  Color
        r = m_ScriptEngine->RegisterObjectType ("Color", sizeof (ALLEGRO_COLOR),
                                                asOBJ_VALUE | asOBJ_POD | asOBJ_APP_FLOAT);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectProperty ("Color", "float r", asOFFSET (ALLEGRO_COLOR, r));
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectProperty ("Color", "float g", asOFFSET (ALLEGRO_COLOR, g));
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectProperty ("Color", "float b", asOFFSET (ALLEGRO_COLOR, b));
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectProperty ("Color", "float a", asOFFSET (ALLEGRO_COLOR, a));
        assert (r >= 0);

        r = m_ScriptEngine->RegisterGlobalProperty ("Color COLOR_BLACK", &COLOR_BLACK_REF);
        assert (r >= 0);

        // Rect
        r = m_ScriptEngine->RegisterObjectType (
            "Rect", sizeof (Rect), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CA | asOBJ_APP_CLASS_ALLFLOATS);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectBehaviour ("Rect", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION (ConstructRect),
                                                     asCALL_CDECL_OBJLAST);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectBehaviour ("Rect", asBEHAVE_CONSTRUCT, "void f(const Rect &in)",
                                                     asFUNCTION (ConstructRectCopy), asCALL_CDECL_OBJLAST);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectBehaviour ("Rect", asBEHAVE_CONSTRUCT, "void f(Point, Point)",
                                                     asFUNCTION (ConstructRectXY), asCALL_CDECL_OBJLAST);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("Rect", "void SetPos (Point)",
                                                  asMETHODPR (Rect, SetPos, (Point), void), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("Rect", "Point GetPos ()", asMETHOD (Rect, GetPos), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("Rect", "void SetSize (Point)",
                                                  asMETHODPR (Rect, SetSize, (Point), void), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("Rect", "Point GetSize ()", asMETHOD (Rect, GetSize),
                                                  asCALL_THISCALL);
        assert (r >= 0);

        // FlagPoint
        r = m_ScriptEngine->RegisterObjectType ("FlagPoint", 0, asOBJ_REF | asOBJ_NOCOUNT);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectProperty ("FlagPoint", "Point Pos", asOFFSET (FlagPoint, Pos));
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectProperty ("FlagPoint", "string Name", asOFFSET (FlagPoint, Name));
        assert (r >= 0);

        //        //  Scene
        //        r = m_ScriptEngine->RegisterObjectType ("SceneManager", sizeof (SceneManager), asOBJ_VALUE |
        //        asOBJ_POD); assert (r >= 0); r = m_ScriptEngine->RegisterGlobalProperty ("SceneManager sceneManager",
        //        &m_MainLoop->GetSceneManager ()); assert (r >= 0); r = m_ScriptEngine->RegisterObjectMethod
        //        ("SceneManager", "Point GetFlagPoint (const string &in)",
        //            asMETHOD (SceneManager, GetFlagPoint), asCALL_THISCALL);

        r = m_ScriptEngine->RegisterGlobalFunction ("FlagPoint@ GetFlagPoint (const string &in)",
                                                    asMETHOD (SceneManager, GetFlagPoint), asCALL_THISCALL_ASGLOBAL,
                                                    &m_MainLoop->GetSceneManager ());
        assert (r >= 0);

        //  Player
        r = m_ScriptEngine->RegisterObjectType ("Player", sizeof (Player), asOBJ_VALUE | asOBJ_POD);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalProperty ("Player player", &m_MainLoop->GetSceneManager ().GetPlayer ());
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("Player", "void SetPosition (Point)",
                                                  asMETHODPR (Player, SetPosition, (Point), void), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("Player", "void SetPosition (float, float)",
                                                  asMETHODPR (Player, SetPosition, (float, float), void),
                                                  asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("Player", "Point GetPosition ()", asMETHOD (Player, GetPosition),
                                                  asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("Player", "Point GetSize ()", asMETHOD (Player, GetSize),
                                                  asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("Player", "void Move (float, float)", asMETHOD (Player, Move),
                                                  asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("Player", "void SetFollowCamera (bool)",
                                                  asMETHOD (Player, SetFollowCamera), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("Player", "void SetCurrentAnimation (const string &in)",
                                                  asMETHOD (Player, SetCurrentAnimation), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("Player", "void SetPreventInput (bool)",
                                                  asMETHOD (Player, SetPreventInput), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("Player", "bool IsPreventInput ()", asMETHOD (Player, IsPreventInput),
                                                  asCALL_THISCALL);
        assert (r >= 0);

        //  Camera
        r = m_ScriptEngine->RegisterObjectType ("Camera", sizeof (Camera), asOBJ_VALUE | asOBJ_POD);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalProperty ("Camera camera", &m_MainLoop->GetSceneManager ().GetCamera ());
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("Camera", "void SetTranslate (float dx, float dy)",
                                                  asMETHOD (Camera, SetTranslate), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("Camera", "Point GetScale ()", asMETHOD (Camera, GetScale),
                                                  asCALL_THISCALL);
        assert (r >= 0);

        //  Screen
        r = m_ScriptEngine->RegisterObjectType ("Screen", sizeof (Screen), asOBJ_VALUE | asOBJ_POD);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalProperty ("Screen screen", m_MainLoop->GetScreen ());
        assert (r >= 0);

        //  Trigger Area
        r = m_ScriptEngine->RegisterFuncdef ("void TriggerFunc (Point)");
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction (
            "void AddOnEnterCallback (const string &in, TriggerFunc @tf)",
            asMETHODPR (SceneManager, AddOnEnterCallback, (const std::string&, asIScriptFunction*), void),
            asCALL_THISCALL_ASGLOBAL, &m_MainLoop->GetSceneManager ());
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction (
            "void AddOnLeaveCallback (const string &in, TriggerFunc @tf)",
            asMETHODPR (SceneManager, AddOnLeaveCallback, (const std::string&, asIScriptFunction*), void),
            asCALL_THISCALL_ASGLOBAL, &m_MainLoop->GetSceneManager ());
        assert (r >= 0);

        //  Speech Frame
        r = m_ScriptEngine->RegisterObjectType ("SpeechFrame", 0, asOBJ_REF | asOBJ_NOCOUNT);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("SpeechFrame", "void Show ()", asMETHOD (SpeechFrame, Show),
                                                  asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("SpeechFrame", "void Hide ()", asMETHOD (SpeechFrame, Hide),
                                                  asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("SpeechFrame", "bool IsVisible ()", asMETHOD (SpeechFrame, IsVisible),
                                                  asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("SpeechFrame", "void SetDrawRect (Rect)",
                                                  asMETHOD (SpeechFrame, SetDrawRect), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("SpeechFrame", "void SetDrawTextCenter (bool)",
                                                  asMETHOD (SpeechFrame, SetDrawTextCenter), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("SpeechFrame", "bool IsDrawTextCenter ()",
                                                  asMETHOD (SpeechFrame, IsDrawTextCenter), asCALL_THISCALL);
        assert (r >= 0);

        //  Speech Frame Manager
        r = m_ScriptEngine->RegisterGlobalFunction (
            "SpeechFrame@ AddSpeechFrame (const string &in, const string &in, "
            "Rect, bool = true, const string &in = \"\")",
            asMETHODPR (SpeechFrameManager, AddSpeechFrame,
                        (const std::string&, const std::string&, Rect, bool, const std::string& regionName),
                        SpeechFrame*),
            asCALL_THISCALL_ASGLOBAL, &m_MainLoop->GetSceneManager ().GetSpeechFrameManager ());
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction (
            "SpeechFrame@ AddSpeechFrame (const string &in, const string &in, "
            "Point, int, int, bool = true, const string &in = \"\")",
            asMETHODPR (SpeechFrameManager, AddSpeechFrame,
                        (const std::string&, const std::string&, Point, int, int, bool, const std::string& regionName),
                        SpeechFrame*),
            asCALL_THISCALL_ASGLOBAL, &m_MainLoop->GetSceneManager ().GetSpeechFrameManager ());
        assert (r >= 0);

        // Scene Manager
        r = m_ScriptEngine->RegisterGlobalFunction (
            "void SetActiveScene (const string &in, bool fadeAnim = true)",
            asMETHODPR (SceneManager, SetActiveScene, (const std::string&, bool), void), asCALL_THISCALL_ASGLOBAL,
            &m_MainLoop->GetSceneManager ());
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction (
            "void SceneFadeInOut (float fadeInMs = 500, float fadeOutMs = 500, Color color = COLOR_BLACK)",
            asMETHOD (SceneManager, SceneFadeInOut), asCALL_THISCALL_ASGLOBAL, &m_MainLoop->GetSceneManager ());
        assert (r >= 0);

        //  Global
        r = m_ScriptEngine->RegisterGlobalFunction ("void Log(const string &in)",
                                                    asFUNCTIONPR (Log, (const std::string&), void), asCALL_CDECL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction ("void Log (float)", asFUNCTIONPR (Log, (float), void),
                                                    asCALL_CDECL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction ("const Point& GetWindowSize ()", asMETHOD (Screen, GetWindowSize),
                                                    asCALL_THISCALL_ASGLOBAL, m_MainLoop->GetScreen ());
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction ("float GetDeltaTime ()", asMETHOD (Screen, GetDeltaTime),
                                                    asCALL_THISCALL_ASGLOBAL, m_MainLoop->GetScreen ());
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction ("float GetFPS ()", asMETHOD (Screen, GetFPS),
                                                    asCALL_THISCALL_ASGLOBAL, m_MainLoop->GetScreen ());
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction ("void SetBackgroundColor (Color)",
                                                    asMETHODPR (Screen, SetBackgroundColor, (ALLEGRO_COLOR), void),
                                                    asCALL_THISCALL_ASGLOBAL, m_MainLoop->GetScreen ());
        assert (r >= 0);

        //  Tweening
        //        r = m_ScriptEngine->RegisterFuncdef ("bool TweenFuncFloat (float, float)");
        //        assert (r >= 0);
        //        r = m_ScriptEngine->RegisterGlobalFunction ("void AddTween (int, float, float, int, TweenFuncFloat
        //        @tf)",
        //            asMETHODPR (TweenManager, AddTween, (int, float, float, int, asIScriptFunction*), void),
        //            asCALL_THISCALL_ASGLOBAL, &m_MainLoop->GetTweenManager ());
        //        assert (r >= 0);

        r = m_ScriptEngine->RegisterFuncdef ("bool TweenFuncPoint (int id, float progress, Point value)");
        assert (r >= 0);
        r = m_ScriptEngine->RegisterFuncdef ("void TweenFuncPointFinish (int)");
        assert (r >= 0);

        r = m_ScriptEngine->RegisterGlobalFunction (
            "void AddTween (int, Point, Point, int, TweenFuncPoint @tf, TweenFuncPointFinish @te)",
            asMETHODPR (TweenManager, AddTween, (int, Point, Point, int, asIScriptFunction*, asIScriptFunction*), void),
            asCALL_THISCALL_ASGLOBAL, &m_MainLoop->GetTweenManager ());
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction ("void PauseTween (int)", asMETHOD (TweenManager, PauseTween),
                                                    asCALL_THISCALL_ASGLOBAL, &m_MainLoop->GetTweenManager ());
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction ("void ResumeTween (int)", asMETHOD (TweenManager, ResumeTween),
                                                    asCALL_THISCALL_ASGLOBAL, &m_MainLoop->GetTweenManager ());
        assert (r >= 0);
    }

    //--------------------------------------------------------------------------------------------------

    asIScriptEngine* ScriptManager::GetEngine () { return m_ScriptEngine; }

    //--------------------------------------------------------------------------------------------------

    Script* ScriptManager::GetScriptByModuleName (const std::string& moduleName) { return m_Scripts[moduleName]; }

    //--------------------------------------------------------------------------------------------------
}
