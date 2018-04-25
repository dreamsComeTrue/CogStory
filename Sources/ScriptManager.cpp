// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "ScriptManager.h"
#include "MainLoop.h"
#include "Scene.h"
#include "SceneManager.h"
#include "Screen.h"
#include "Script.h"
#include "SpeechFrame.h"
#include "SpeechFrameManager.h"

#include <chrono>

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    std::string lastWatchedFile;
    std::chrono::system_clock::time_point lastTimePoint;

    void FileUpdateListener::handleFileAction (FW::WatchID watchid, const FW::String& dir, const FW::String& fileName,
                                               FW::Action action)
    {
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now ();

        auto fraction = now - lastTimePoint;
        auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds> (fraction);
        if (milliseconds.count () < 500)
        {
            return;
        }

        auto reloadScriptsFunc = [&](Scriptable* scriptable) {
            for (ScriptMetaData& script : scriptable->GetScripts ())
            {
                std::string file = fileName;
                std::replace (file.begin (), file.end (), '\\', '/');

                if (script.Path == file)
                {
                    scriptable->ReloadScript (script.Name);
                }
            }
        };

        reloadScriptsFunc (m_SceneManager->GetActiveScene ());

        std::vector<Actor*>& actors = m_SceneManager->GetActiveScene ()->GetActors ();

        for (Actor* actor : actors)
        {
            reloadScriptsFunc (actor);
        }

        lastTimePoint = now;
    }

    //--------------------------------------------------------------------------------------------------
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

        Log (8000.f, COLOR_ORANGE, "%s", g_ScriptErrorBuffer);
    }

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

        Log (8000.f, COLOR_ORANGE, "%s", buffer);
    }

    //--------------------------------------------------------------------------------------------------

    ScriptManager::ScriptManager (MainLoop* mainLoop)
        : m_FileListener (&mainLoop->GetSceneManager ())
        , m_MainLoop (mainLoop)
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

        m_FileWatcher.addWatch (GetDataPath () + "/scripts", &m_FileListener, true);

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool ScriptManager::Destroy ()
    {
        for (std::map<std::string, Script*>::iterator it = m_Scripts.begin (); it != m_Scripts.end (); ++it)
        {
            if (it->second)
            {
                SAFE_DELETE (it->second);
            }
        }

        m_ScriptEngine->ShutDownAndRelease ();

        return Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    void ScriptManager::Update (float deltaTime) { m_FileWatcher.update (); }

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
            Log (8000.f, COLOR_ORANGE, "Unrecoverable error while starting a new module.\n");
            return nullptr;
        }

        r = builder.AddSectionFromMemory (moduleName.c_str (), text.c_str ());

        if (r < 0)
        {
            // The builder wasn't able to load the file. Maybe the file
            // has been removed, or the wrong name was given, or some
            // preprocessing commands are incorrectly written.
            Log (8000.f, COLOR_ORANGE, "Please correct the errors in the script and try again.\n");
            return nullptr;
        }

        r = builder.BuildModule ();

        if (r < 0)
        {
            // An error occurred. Instruct the script writer to fix the
            // compilation errors that were listed in the output stream.
            Log (8000.f, COLOR_ORANGE, "Please correct the errors in the script and try again.\n");
            return nullptr;
        }

        Script* script = new Script (builder.GetModule (), this, moduleName);
        m_Scripts.insert (std::make_pair (moduleName, script));

        script->Initialize ();

        return script;
    }

    //--------------------------------------------------------------------------------------------------

    void ScriptManager::RemoveScript (const std::string& name)
    {
        for (std::map<std::string, Script*>::iterator it = m_Scripts.begin (); it != m_Scripts.end (); ++it)
        {
            if (it->second->GetName () == name)
            {
                SAFE_DELETE (it->second);
                m_Scripts.erase (it);
                return;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void ScriptManager::RemoveScript (Script* script) { RemoveScript (script->GetName ()); }

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

    static void Log (const std::string& data) { Log ("%s\n", data.c_str ()); }

    //--------------------------------------------------------------------------------------------------

    static void Log (Point point) { Log ("Point [X = %f, Y = %f]\n", point.X, point.Y); }

    //--------------------------------------------------------------------------------------------------

    static void Log (float data) { Log ("%f\n", data); }

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

        r = m_ScriptEngine->RegisterGlobalFunction ("void Log (Point)", asFUNCTIONPR (Log, (Point), void),
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
            "void AddTween (int, Point, Point, int, TweenFuncPoint @+ tf, TweenFuncPointFinish @+ te)",
            asMETHODPR (TweenManager, AddTween, (int, Point, Point, int, asIScriptFunction*, asIScriptFunction*), void),
            asCALL_THISCALL_ASGLOBAL, &m_MainLoop->GetTweenManager ());
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction ("void PauseTween (int)", asMETHOD (TweenManager, PauseTween),
                                                    asCALL_THISCALL_ASGLOBAL, &m_MainLoop->GetTweenManager ());
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction ("void ResumeTween (int)", asMETHOD (TweenManager, ResumeTween),
                                                    asCALL_THISCALL_ASGLOBAL, &m_MainLoop->GetTweenManager ());
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
        r = m_ScriptEngine->RegisterGlobalProperty ("Player player", m_MainLoop->GetSceneManager ().GetPlayer ());
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
        r = m_ScriptEngine->RegisterObjectMethod ("Player", "void SetCurrentAnimation (const string &in)",
                                                  asMETHOD (Player, SetCurrentAnimation), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("Player", "void SetPreventInput (bool)",
                                                  asMETHOD (Player, SetPreventInput), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("Player", "bool IsPreventInput ()", asMETHOD (Player, IsPreventInput),
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
            "void AddOnEnterCallback (const string &in, TriggerFunc @+ tf)",
            asMETHODPR (SceneManager, AddOnEnterCallback, (const std::string&, asIScriptFunction*), void),
            asCALL_THISCALL_ASGLOBAL, &m_MainLoop->GetSceneManager ());
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction (
            "void RemoveOnEnterCallback (const string &in)",
            asMETHODPR (SceneManager, RemoveOnEnterCallback, (const std::string&), void), asCALL_THISCALL_ASGLOBAL,
            &m_MainLoop->GetSceneManager ());
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction (
            "void AddOnLeaveCallback (const string &in, TriggerFunc @+ tf)",
            asMETHODPR (SceneManager, AddOnLeaveCallback, (const std::string&, asIScriptFunction*), void),
            asCALL_THISCALL_ASGLOBAL, &m_MainLoop->GetSceneManager ());
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction (
            "void RemoveOnLeaveCallback (const string &in)",
            asMETHODPR (SceneManager, RemoveOnLeaveCallback, (const std::string&), void), asCALL_THISCALL_ASGLOBAL,
            &m_MainLoop->GetSceneManager ());
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

        //  Actor
        r = m_ScriptEngine->RegisterObjectType ("Actor", 0, asOBJ_REF | asOBJ_NOCOUNT);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("Actor", "void Move (float, float)", asMETHOD (Actor, Move),
                                                  asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("Actor", "void SetPosition (float, float)",
                                                  asMETHODPR (Actor, SetPosition, (float, float), void),
                                                  asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("Actor", "void SetPosition (Point)",
                                                  asMETHODPR (Actor, SetPosition, (Point), void), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("Actor", "Point GetPosition ()", asMETHOD (Actor, GetPosition),
                                                  asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction ("Actor@ GetCurrentActor ()",
                                                    asMETHOD (SceneManager, GetCurrentlyProcessedActor),
                                                    asCALL_THISCALL_ASGLOBAL, &m_MainLoop->GetSceneManager ());
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
        r = m_ScriptEngine->RegisterGlobalFunction ("Actor@ GetActor (const string &in)",
                                                    asMETHOD (SceneManager, GetActor), asCALL_THISCALL_ASGLOBAL,
                                                    &m_MainLoop->GetSceneManager ());
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
        r = m_ScriptEngine->RegisterObjectMethod ("Camera", "void SetCenter (float, float)",
                                                  asMETHOD (Camera, SetCenter), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("Camera",
                                                  "void SetFollowActor (Actor@, Point followOffset = Point(0.f, 0.f))",
                                                  asMETHOD (Camera, SetFollowActor), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "Camera", "void TweenToPoint (Point point, float timeMs = 1000, bool centerScreen = true)",
            asMETHODPR (Camera, TweenToPoint, (Point, float, bool), void), asCALL_THISCALL);
        assert (r >= 0);

        r = m_ScriptEngine->RegisterObjectMethod (
            "Camera",
            "void TweenToPoint (Point point, TweenFuncPointFinish @+ te, float timeMs = 1000, bool centerScreen = "
            "true)",
            asMETHODPR (Camera, TweenToPoint, (Point, asIScriptFunction*, float, bool), void), asCALL_THISCALL);
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
    }

    //--------------------------------------------------------------------------------------------------

    asIScriptEngine* ScriptManager::GetEngine () { return m_ScriptEngine; }

    //--------------------------------------------------------------------------------------------------

    Script* ScriptManager::GetScriptByModuleName (const std::string& moduleName) { return m_Scripts[moduleName]; }

    //--------------------------------------------------------------------------------------------------

    asIScriptContext* ScriptManager::GetContext ()
    {
        asIScriptContext* ctx = m_ScriptEngine->RequestContext ();
        ctx->SetExceptionCallback (asFUNCTION (ExceptionCallback), this, asCALL_THISCALL);

        return ctx;
    }

    //--------------------------------------------------------------------------------------------------
}
