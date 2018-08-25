// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "ScriptManager.h"
#include "AudioManager.h"
#include "AudioSample.h"
#include "MainLoop.h"
#include "Scene.h"
#include "SceneManager.h"
#include "Screen.h"
#include "Script.h"
#include "SpeechFrame.h"
#include "SpeechFrameManager.h"
#include "actors/components/MovementComponent.h"

#include <chrono>

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    /*
       === API Documentation ===

       Point
           Point ()
           Point (float, float)
           Log (Point)
           float X
           float Y
           float Width
           float Height

       Color
           float r
           float g
           float b
           float a
           Color COLOR_BLACK

       Rect
           Rect ()
           Rect (Point, Point)
           SetPos (Point)
           Point GetPos ()
           SetSize (Point)
           Point GetSize ()
           Point GetHalfSize ()

       Tween
               = bool TweenFuncPoint (int id, float progress, Point value)
               = void TweenFuncPointFinish (int)
           void AddTween (int, Point, Point, int, TweenFuncPoint @+ tf, TweenFuncPointFinish @+ te)
           void PauseTween (int)
           void ResumeTween (int)

       FlagPoint
           Point Pos
           string Name
           FlagPoint@ GetFlagPoint (const string &in)

       Player
           Player player
           Rect Bounds
           void SetPosition (Point)
           void SetPosition (float, float)
           Point GetPosition ()
           Point GetSize ()
           void Move (float, float)
           void SetCurrentAnimation (const string &in)
           void SetPreventInput (bool)
           bool IsPreventInput ()
           bool IsAction ()
            = void ActionHandler ()
           void SetActionHandler (ActionHandler @+ callback)

       Screen
           Screen screen
           const Point& GetWindowSize ()

       AudioManager
           AudioSample@ LoadSampleFromFile (const std::string& sampleName, const std::string& path);
           AudioSample@ GetSample (const std::string& sampleName);
           void RemoveSample (const std::string& sampleName);
           void SetEnabled (bool enabled);
           bool IsEnabled ();

       AudioSample
           void Play ()
           void Stop ()
           void Pause ()
           void Resume ()
           void SetVolume (float volume)
           void SetLooping (bool loop)
           bool IsLooping () const
           void SetFadeIn (float milliseconds)
           void SetFadeOut (float milliseconds, bool pauseOnFinish)

       TriggerArea
               = void TriggerFunc (Point)
           void AddOnEnterCallback (const string &in, TriggerFunc @+ tf)
           void RemoveOnEnterCallback (const string &in)
           void AddOnLeaveCallback (const string &in, TriggerFunc @+ tf)
           void RemoveOnLeaveCallback (const string &in)

       SpeechFrame
           void Show ()
           void Hide ()
           bool IsVisible ()
           void SetDrawRect (Rect)
           void SetDrawTextCenter (bool)
           bool IsDrawTextCenter ()
               = void ScriptHandleFunc ()
           void SetHandleFunction (ScriptHandleFunc)

       SpeechFrameManager
           SpeechFrame@ AddSpeechFrame (const string &in, const string &in, Rect, bool = true, const string &in = "")
           SpeechFrame@ AddSpeechFrame (const string &in, const string &in, Point, int, int, bool = true, const string
       &in="")
               = void SpeechHandler ()
           void RegisterSpeechFinishedHandler (SpeechHandler @+ hd)

       Actor
           Rect Bounds
           void Move (float, float)
           void SetPosition (float, float)
           void SetPosition (Point)
           Point GetPosition ()
           Point GetSize ()
           string GetTypeName ()
           void SetCurrentAnimation (const string &in)
           Actor@ GetCurrentActor ()
           void AddCollisionCallback (Actor@ actor)
           bool IsOverlaping (Actor* actor)
           void OrientTo (Actor@)
           void SuspendUpdate ()
           void ResumeUpdate ()

       MovementComponent
           MovementType
               MoveHorizontal
               MoveVertical
               MovePoints
               MoveWander
           void SetEnabled (bool enabled)
           bool IsEnabled () const
           MovementComponent@ GetMovementComponent (const string &in)
           void SetMovementType (MovementType type)
           MovementType GetMovementType ()
           void SetMoveExtents (Point min, Point max)
           void SetSpeed (float speed)
           float GetSpeed ()
           void SetWaitLikelihood (float percentage)
           void SetWalkPoints (array<Point>@+ points)
           void SetWalkPoints (FlagPoint@ points)
               = void MovementCallback (Point)
           void SetMovementCallback (MovementCallback @+ mc)

       SceneManager
           void SetActiveScene (const string &in, bool fadeAnim = true)
           void SceneFadeInOut (float fadeInMs = 500, float fadeOutMs = 500, Color color = COLOR_BLACK)
           void SetSuppressSceneInfo (bool suppress)
           bool IsSuppressSceneInfo () const
           void SetOverlayText (const string& in, float duration = 2000.f, float charTimeDelay = 5.f,
                ScreenRelativePosition pos = BottomRight)
           Actor@ GetActor (const string &in)
           string ChoiceFunction (void)
           void RegisterChoiceFunction (string, ChoiceFunction @+ func)
           AudioSample@ SetSceneAudioStream (const string &in path)
           AudioSample@ GetSceneAudioStream ()

       Camera
           Camera camera
           void SetTranslate (float dx, float dy)
           Point GetScale ()
           void SetCenter (float, float)
           void SetFollowActor (Actor@, Point followOffset = Point(0.f, 0.f))
           void TweenToPoint (Point point, float timeMs = 1000, bool centerScreen = true)
           void TweenToPoint (Point point, TweenFuncPointFinish @+ te, float timeMs = 1000, bool centerScreen = true)

       Global
           void Log(const string &in)
           void Log (float)
           const Point& GetWindowSize ()
           float GetDeltaTime ()
           float GetFPS ()
           void SetBackgroundColor (Color)
     */

    std::string lastWatchedFile;
    std::chrono::system_clock::time_point lastTimePoint;

    void FileUpdateListener::handleFileAction (
        FW::WatchID watchid, const FW::String& dir, const FW::String& fileName, FW::Action action)
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

    template <class A, class B> B* RefCast (A* a)
    {
        // If the handle already is a null handle, then just return the null handle
        if (!a)
        {
            return nullptr;
        }

        // Now try to dynamically cast the pointer to the wanted type
        B* b = dynamic_cast<B*> (a);
        if (b != nullptr)
        {
            // Since the cast was made, we need to increase the ref counter for the returned handle
            // b->addref ();
        }

        return b;
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
        RegisterScriptArray (m_ScriptEngine, true);
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
        //  Enums
        RegisterEnums ();

        //  Point
        RegisterPointAPI ();

        //  Color
        RegisterColorAPI ();

        // Rect
        RegisterRectAPI ();

        //  Tween
        RegisterTweenAPI ();

        // FlagPoint
        RegisterFlagPointAPI ();

        //  Actor
        RegisterActorAPI ();

        //  Player
        RegisterPlayerAPI ();

        //  Screen
        RegisterScreenAPI ();

        //  Audio Sample
        RegisterAudioSampleAPI ();

        //  Audio Manager
        RegisterAudioManagerAPI ();

        //  Trigger Area
        RegisterTriggerAreaAPI ();

        //  Speech Frame
        RegisterSpeechFrameAPI ();

        //  Speech Frame Manager
        RegisterSpeechFrameManagerAPI ();

        //  MovementComponent
        RegisterMovementComponentAPI ();

        // Scene Manager
        RegisterSceneManagerAPI ();

        // Scene
        RegisterSceneAPI ();

        //  Camera
        RegisterCameraAPI ();

        //  Global
        RegisterGlobalAPI ();
    }

    //--------------------------------------------------------------------------------------------------

    void ScriptManager::RegisterEnums ()
    {
        int r = m_ScriptEngine->RegisterEnum ("MovementType");
        assert (r >= 0);
        r = m_ScriptEngine->RegisterEnumValue ("MovementType", "MoveHorizontal", MovementType::MoveHorizontal);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterEnumValue ("MovementType", "MoveVertical", MovementType::MoveVertical);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterEnumValue ("MovementType", "MovePoints", MovementType::MovePoints);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterEnumValue ("MovementType", "MoveWander", MovementType::MoveWander);
        assert (r >= 0);

        r = m_ScriptEngine->RegisterEnum ("ScreenRelativePosition");
        assert (r >= 0);
        r = m_ScriptEngine->RegisterEnumValue ("ScreenRelativePosition", "Absoulte", ScreenRelativePosition::Absoulte);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterEnumValue ("ScreenRelativePosition", "TopLeft", ScreenRelativePosition::TopLeft);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterEnumValue (
            "ScreenRelativePosition", "TopCenter", ScreenRelativePosition::TopCenter);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterEnumValue ("ScreenRelativePosition", "TopRight", ScreenRelativePosition::TopRight);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterEnumValue (
            "ScreenRelativePosition", "BottomLeft", ScreenRelativePosition::BottomLeft);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterEnumValue (
            "ScreenRelativePosition", "BottomCenter", ScreenRelativePosition::BottomCenter);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterEnumValue (
            "ScreenRelativePosition", "BottomRight", ScreenRelativePosition::BottomRight);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterEnumValue ("ScreenRelativePosition", "Center", ScreenRelativePosition::Center);
        assert (r >= 0);
    }

    //--------------------------------------------------------------------------------------------------

    void ScriptManager::RegisterPointAPI ()
    {
        int r = m_ScriptEngine->RegisterObjectType (
            "Point", sizeof (Point), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CA | asOBJ_APP_CLASS_ALLFLOATS);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectBehaviour (
            "Point", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION (ConstructPoint), asCALL_CDECL_OBJLAST);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectBehaviour ("Point", asBEHAVE_CONSTRUCT, "void f(const Point &in)",
            asFUNCTION (ConstructPointCopy), asCALL_CDECL_OBJLAST);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectBehaviour (
            "Point", asBEHAVE_CONSTRUCT, "void f(float, float)", asFUNCTION (ConstructPointXY), asCALL_CDECL_OBJLAST);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectProperty ("Point", "float X", asOFFSET (Point, X));
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectProperty ("Point", "float Y", asOFFSET (Point, Y));
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectProperty ("Point", "float Width", asOFFSET (Point, Width));
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectProperty ("Point", "float Height", asOFFSET (Point, Height));
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("Point", "Point opAdd(const Point &in)",
            asMETHODPR (Point, operator+, (const Point&) const, Point), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("Point", "Point opSub(const Point &in)",
            asMETHODPR (Point, operator-, (const Point&) const, Point), asCALL_THISCALL);
        assert (r >= 0);

        r = m_ScriptEngine->RegisterGlobalFunction (
            "void Log (Point)", asFUNCTIONPR (Log, (Point), void), asCALL_CDECL);
        assert (r >= 0);
    }

    //--------------------------------------------------------------------------------------------------

    void ScriptManager::RegisterColorAPI ()
    {
        int r = m_ScriptEngine->RegisterObjectType (
            "Color", sizeof (ALLEGRO_COLOR), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_FLOAT);
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
    }

    //--------------------------------------------------------------------------------------------------

    void ScriptManager::RegisterRectAPI ()
    {
        int r = m_ScriptEngine->RegisterObjectType (
            "Rect", sizeof (Rect), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CA | asOBJ_APP_CLASS_ALLFLOATS);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectBehaviour (
            "Rect", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION (ConstructRect), asCALL_CDECL_OBJLAST);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectBehaviour (
            "Rect", asBEHAVE_CONSTRUCT, "void f(const Rect &in)", asFUNCTION (ConstructRectCopy), asCALL_CDECL_OBJLAST);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectBehaviour (
            "Rect", asBEHAVE_CONSTRUCT, "void f(Point, Point)", asFUNCTION (ConstructRectXY), asCALL_CDECL_OBJLAST);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "Rect", "void SetPos (Point)", asMETHODPR (Rect, SetPos, (Point), void), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("Rect", "Point GetPos ()", asMETHOD (Rect, GetPos), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "Rect", "void SetSize (Point)", asMETHODPR (Rect, SetSize, (Point), void), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "Rect", "Point GetSize ()", asMETHOD (Rect, GetSize), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "Rect", "Point GetHalfSize ()", asMETHOD (Rect, GetHalfSize), asCALL_THISCALL);
        assert (r >= 0);
    }

    //--------------------------------------------------------------------------------------------------

    void ScriptManager::RegisterTweenAPI ()
    {
        int r = m_ScriptEngine->RegisterFuncdef ("bool TweenFuncPoint (int id, float progress, Point value)");
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
    }

    //--------------------------------------------------------------------------------------------------

    void ScriptManager::RegisterFlagPointAPI ()
    {
        int r = m_ScriptEngine->RegisterObjectType ("FlagPoint", 0, asOBJ_REF | asOBJ_NOCOUNT);
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
            asMETHOD (SceneManager, GetFlagPoint), asCALL_THISCALL_ASGLOBAL, &m_MainLoop->GetSceneManager ());
        assert (r >= 0);
    }

    //--------------------------------------------------------------------------------------------------

    void ScriptManager::RegisterPlayerAPI ()
    {
        // int r = m_ScriptEngine->RegisterObjectType ("Player", sizeof (Player), asOBJ_VALUE | asOBJ_POD);
        int r = m_ScriptEngine->RegisterObjectType ("Player", 0, asOBJ_REF | asOBJ_NOCOUNT);
        assert (r >= 0);

        r = m_ScriptEngine->RegisterObjectMethod (
            "Player", "Actor@ opImplCast()", asFUNCTION ((RefCast<Player, Actor>)), asCALL_CDECL_OBJLAST);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "Player", "Actor@ opCast()", asFUNCTION ((RefCast<Player, Actor>)), asCALL_CDECL_OBJLAST);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "Actor", "Player@ opImplCast()", asFUNCTION ((RefCast<Actor, Player>)), asCALL_CDECL_OBJLAST);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "Actor", "Player@ opCast()", asFUNCTION ((RefCast<Actor, Player>)), asCALL_CDECL_OBJLAST);
        assert (r >= 0);

        r = m_ScriptEngine->RegisterGlobalProperty ("Player player", m_MainLoop->GetSceneManager ().GetPlayer ());
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectProperty ("Player", "Rect Bounds", asOFFSET (Player, Bounds));
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "Player", "void SetPosition (Point)", asMETHODPR (Player, SetPosition, (Point), void), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("Player", "void SetPosition (float, float)",
            asMETHODPR (Player, SetPosition, (float, float), void), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "Player", "Point GetPosition ()", asMETHOD (Player, GetPosition), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "Player", "Point GetSize ()", asMETHOD (Player, GetSize), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "Player", "void Move (float, float)", asMETHODPR (Player, Move, (float, float), void), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("Player", "void SetCurrentAnimation (const string &in)",
            asMETHOD (Player, SetCurrentAnimation), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "Player", "void SetPreventInput (bool)", asMETHOD (Player, SetPreventInput), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "Player", "bool IsPreventInput ()", asMETHOD (Player, IsPreventInput), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "Player", "bool IsAction ()", asMETHOD (Player, IsAction), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterFuncdef ("void ActionHandler ()");
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("Player", "void SetActionHandler (ActionHandler @+ callback)",
            asMETHOD (Player, SetActionHandler), asCALL_THISCALL);
        assert (r >= 0);
    }

    //--------------------------------------------------------------------------------------------------

    void ScriptManager::RegisterScreenAPI ()
    {
        int r = m_ScriptEngine->RegisterObjectType ("Screen", sizeof (Screen), asOBJ_VALUE | asOBJ_POD);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalProperty ("Screen screen", m_MainLoop->GetScreen ());
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "Screen", "const Point& GetWindowSize ()", asMETHOD (Screen, GetWindowSize), asCALL_THISCALL);
        assert (r >= 0);
    }

    //--------------------------------------------------------------------------------------------------

    void ScriptManager::RegisterAudioSampleAPI ()
    {
        int r = m_ScriptEngine->RegisterObjectType ("AudioSample", 0, asOBJ_REF | asOBJ_NOCOUNT);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "AudioSample", "void Stop ()", asMETHOD (AudioSample, Stop), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "AudioSample", "void Play ()", asMETHOD (AudioSample, Play), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "AudioSample", "void Pause ()", asMETHOD (AudioSample, Pause), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "AudioSample", "void Resume ()", asMETHOD (AudioSample, Resume), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "AudioSample", "void SetVolume (float volume = 1.0f)", asMETHOD (AudioSample, SetVolume), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "AudioSample", "void SetLooping (bool loop)", asMETHOD (AudioSample, SetLooping), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "AudioSample", "bool IsLooping () const", asMETHOD (AudioSample, IsLooping), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "AudioSample", "void SetFadeIn (float milliseconds)", asMETHOD (AudioSample, SetFadeIn), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("AudioSample",
            "void SetFadeOut (float milliseconds,"
            "bool pauseOnFinish = true)",
            asMETHOD (AudioSample, SetFadeOut), asCALL_THISCALL);
        assert (r >= 0);
    }

    //--------------------------------------------------------------------------------------------------

    void ScriptManager::RegisterAudioManagerAPI ()
    {
        int r = m_ScriptEngine->RegisterObjectType ("AudioManager", sizeof (AudioManager), asOBJ_VALUE | asOBJ_POD);
        assert (r >= 0);

        r = m_ScriptEngine->RegisterGlobalProperty ("AudioManager audioManager", &m_MainLoop->GetAudioManager ());
        assert (r >= 0);

        r = m_ScriptEngine->RegisterObjectMethod ("AudioManager",
            "AudioSample@ LoadSampleFromFile "
            "(const string &in sampleName, const string &in path)",
            asMETHOD (AudioManager, LoadSampleFromFile), asCALL_THISCALL);
        assert (r >= 0);

        r = m_ScriptEngine->RegisterObjectMethod (
            "AudioManager", "void SetEnabled (bool enabled)", asMETHOD (AudioManager, SetEnabled), asCALL_THISCALL);
        assert (r >= 0);

        r = m_ScriptEngine->RegisterObjectMethod (
            "AudioManager", "bool IsEnabled ()", asMETHOD (AudioManager, IsEnabled), asCALL_THISCALL);
        assert (r >= 0);
    }

    //--------------------------------------------------------------------------------------------------

    void ScriptManager::RegisterTriggerAreaAPI ()
    {
        int r = m_ScriptEngine->RegisterFuncdef ("void TriggerFunc (Point)");
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction ("void AddOnEnterCallback (const string &in, TriggerFunc @+ tf)",
            asMETHODPR (SceneManager, AddOnEnterCallback, (const std::string&, asIScriptFunction*), void),
            asCALL_THISCALL_ASGLOBAL, &m_MainLoop->GetSceneManager ());
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction ("void RemoveOnEnterCallback (const string &in)",
            asMETHODPR (SceneManager, RemoveOnEnterCallback, (const std::string&), void), asCALL_THISCALL_ASGLOBAL,
            &m_MainLoop->GetSceneManager ());
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction ("void AddOnLeaveCallback (const string &in, TriggerFunc @+ tf)",
            asMETHODPR (SceneManager, AddOnLeaveCallback, (const std::string&, asIScriptFunction*), void),
            asCALL_THISCALL_ASGLOBAL, &m_MainLoop->GetSceneManager ());
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction ("void RemoveOnLeaveCallback (const string &in)",
            asMETHODPR (SceneManager, RemoveOnLeaveCallback, (const std::string&), void), asCALL_THISCALL_ASGLOBAL,
            &m_MainLoop->GetSceneManager ());
        assert (r >= 0);
    }

    //--------------------------------------------------------------------------------------------------

    void ScriptManager::RegisterSpeechFrameAPI ()
    {
        int r = m_ScriptEngine->RegisterObjectType ("SpeechFrame", 0, asOBJ_REF | asOBJ_NOCOUNT);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "SpeechFrame", "void Show ()", asMETHOD (SpeechFrame, Show), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "SpeechFrame", "void Hide ()", asMETHOD (SpeechFrame, Hide), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "SpeechFrame", "bool IsVisible ()", asMETHOD (SpeechFrame, IsVisible), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "SpeechFrame", "void SetDrawRect (Rect)", asMETHOD (SpeechFrame, SetDrawRect), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "SpeechFrame", "void SetDrawTextCenter (bool)", asMETHOD (SpeechFrame, SetDrawTextCenter), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "SpeechFrame", "bool IsDrawTextCenter ()", asMETHOD (SpeechFrame, IsDrawTextCenter), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterFuncdef ("void ScriptHandleFunc ()");
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("SpeechFrame", "void SetHandleFunction (ScriptHandleFunc @+ tf)",
            asMETHOD (SpeechFrame, SetHandleFunction), asCALL_THISCALL);
        assert (r >= 0);
    }

    //--------------------------------------------------------------------------------------------------

    void ScriptManager::RegisterSpeechFrameManagerAPI ()
    {
        int r = m_ScriptEngine->RegisterGlobalFunction (
            "SpeechFrame@ AddSpeechFrame (const string &in, const string &in, "
            "Rect, bool = true, const string &in = \"\", const string &in = \"\")",
            asMETHODPR (SpeechFrameManager, AddSpeechFrame,
                (const std::string&, const std::string&, Rect, bool, const std::string& actionName,
                    const std::string& regionName),
                SpeechFrame*),
            asCALL_THISCALL_ASGLOBAL, &m_MainLoop->GetSceneManager ().GetSpeechFrameManager ());
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction (
            "SpeechFrame@ AddSpeechFrame (const string &in, const string &in, "
            "Point, int, int, bool = true, const string &in = \"\", const string &in = \"\")",
            asMETHODPR (SpeechFrameManager, AddSpeechFrame,
                (const std::string&, const std::string&, Point, int, int, bool, const std::string& actionName,
                    const std::string& regionName),
                SpeechFrame*),
            asCALL_THISCALL_ASGLOBAL, &m_MainLoop->GetSceneManager ().GetSpeechFrameManager ());
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction ("SpeechFrame@ AddSpeechFrame (const string &in, bool = true)",
            asMETHODPR (SpeechFrameManager, AddSpeechFrame, (const std::string&, bool), SpeechFrame*),
            asCALL_THISCALL_ASGLOBAL, &m_MainLoop->GetSceneManager ().GetSpeechFrameManager ());
        assert (r >= 0);
        r = m_ScriptEngine->RegisterFuncdef ("void SpeechHandler ()");
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction ("void RegisterSpeechFinishedHandler (SpeechHandler @+ hd)",
            asMETHOD (SpeechFrameManager, RegisterSpeechFinishedHandler), asCALL_THISCALL_ASGLOBAL,
            &m_MainLoop->GetSceneManager ().GetSpeechFrameManager ());
        assert (r >= 0);
    }

    //--------------------------------------------------------------------------------------------------

    void ScriptManager::RegisterActorAPI ()
    {
        int r = m_ScriptEngine->RegisterObjectType ("Actor", 0, asOBJ_REF | asOBJ_NOCOUNT);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectProperty ("Actor", "Rect Bounds", asOFFSET (Actor, Bounds));
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "Actor", "void Move (float, float)", asMETHODPR (Actor, Move, (float, float), void), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("Actor", "void SetPosition (float, float)",
            asMETHODPR (Actor, SetPosition, (float, float), void), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "Actor", "void SetPosition (Point)", asMETHODPR (Actor, SetPosition, (Point), void), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "Actor", "Point GetPosition ()", asMETHOD (Actor, GetPosition), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "Actor", "Point GetSize ()", asMETHOD (Actor, GetSize), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("Actor", "void SetCurrentAnimation (const string &in)",
            asMETHOD (Actor, SetCurrentAnimation), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction ("Actor@ GetCurrentActor ()",
            asMETHOD (SceneManager, GetCurrentlyProcessedActor), asCALL_THISCALL_ASGLOBAL,
            &m_MainLoop->GetSceneManager ());
        assert (r >= 0);
        r = m_ScriptEngine->RegisterFuncdef ("void CollisionCbk (Actor@)");
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("Actor", "void AddCollisionCallback (CollisionCbk @+ callback)",
            asMETHODPR (Actor, AddCollisionCallback, (asIScriptFunction*), void), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "Actor", "bool IsOverlaping (Actor@)", asMETHOD (Actor, IsOverlaping), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "Actor", "string GetTypeName ()", asMETHOD (Actor, GetTypeName), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "Actor", "void OrientTo (Actor@)", asMETHOD (Actor, OrientTo), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "Actor", "void SuspendUpdate ()", asMETHOD (Actor, SuspendUpdate), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "Actor", "void ResumeUpdate ()", asMETHOD (Actor, ResumeUpdate), asCALL_THISCALL);
        assert (r >= 0);
    }

    //--------------------------------------------------------------------------------------------------

    void ScriptManager::RegisterMovementComponentAPI ()
    {
        int r = m_ScriptEngine->RegisterObjectType ("MovementComponent", 0, asOBJ_REF | asOBJ_NOCOUNT);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("Actor", "MovementComponent@ GetMovementComponent (const string &in)",
            asMETHOD (Actor, GetMovementComponent), asCALL_THISCALL);
        r = m_ScriptEngine->RegisterObjectMethod ("MovementComponent", "void SetEnabled (bool enabled)",
            asMETHOD (MovementComponent, SetEnabled), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "MovementComponent", "bool IsEnabled () const", asMETHOD (MovementComponent, IsEnabled), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("MovementComponent", "void SetMovementType (MovementType type)",
            asMETHOD (MovementComponent, SetMovementType), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("MovementComponent", "MovementType GetMovementType ()",
            asMETHOD (MovementComponent, GetMovementType), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("MovementComponent", "void SetMoveExtents (Point min, Point max)",
            asMETHOD (MovementComponent, SetMoveExtents), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("MovementComponent", "void SetSpeed (float speed)",
            asMETHOD (MovementComponent, SetSpeed), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "MovementComponent", "float GetSpeed ()", asMETHOD (MovementComponent, GetSpeed), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("MovementComponent", "void SetWaitLikelihood (float percentage)",
            asMETHOD (MovementComponent, SetWaitLikelihood), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("MovementComponent", "void SetWalkPoints (array<Point>@+ points)",
            asMETHODPR (MovementComponent, SetWalkPoints, (CScriptArray*), void), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("MovementComponent", "void SetWalkPoints (FlagPoint@ points)",
            asMETHODPR (MovementComponent, SetWalkPoints, (FlagPoint*), void), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterFuncdef ("void MovementCallback (Point)");
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("MovementComponent",
            "void SetMovementCallback (MovementCallback @+ mc)", asMETHOD (MovementComponent, SetMovementCallback),
            asCALL_THISCALL);
        assert (r >= 0);
    }

    //--------------------------------------------------------------------------------------------------

    void ScriptManager::RegisterSceneManagerAPI ()
    {
        int r = m_ScriptEngine->RegisterGlobalFunction ("void SetActiveScene (const string &in, bool fadeAnim = true)",
            asMETHODPR (SceneManager, SetActiveScene, (const std::string&, bool), void), asCALL_THISCALL_ASGLOBAL,
            &m_MainLoop->GetSceneManager ());
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction (
            "void SceneFadeInOut (float fadeInMs = 500, float fadeOutMs = 500, Color color = COLOR_BLACK)",
            asMETHOD (SceneManager, SceneFadeInOut), asCALL_THISCALL_ASGLOBAL, &m_MainLoop->GetSceneManager ());
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction ("void SetSuppressSceneInfo (bool suppress)",
            asMETHOD (SceneManager, SetSuppressSceneInfo), asCALL_THISCALL_ASGLOBAL, &m_MainLoop->GetSceneManager ());
        assert (r >= 0);
        //        r = m_ScriptEngine->RegisterGlobalFunction ("bool IsSuppressSceneInfo () const",
        //            asMETHOD (SceneManager, IsSuppressSceneInfo), asCALL_THISCALL_ASGLOBAL,
        //            &m_MainLoop->GetSceneManager ());
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction (
            "void SetOverlayText (const string& in, float duration = 2000.f, float charTimeDelay = 5.f, "
            "ScreenRelativePosition pos = BottomRight)",
            asMETHOD (SceneManager, SetOverlayText), asCALL_THISCALL_ASGLOBAL, &m_MainLoop->GetSceneManager ());
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction ("Actor@ GetActor (const string &in)",
            asMETHOD (SceneManager, GetActor), asCALL_THISCALL_ASGLOBAL, &m_MainLoop->GetSceneManager ());
        assert (r >= 0);
        r = m_ScriptEngine->RegisterFuncdef ("string ChoiceFunction (void)");
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction ("void RegisterChoiceFunction (string, ChoiceFunction @+ func)",
            asMETHOD (SceneManager, RegisterChoiceFunction), asCALL_THISCALL_ASGLOBAL, &m_MainLoop->GetSceneManager ());
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction ("AudioSample@ SetSceneAudioStream (const string &in path)",
            asMETHOD (SceneManager, SetSceneAudioStream), asCALL_THISCALL_ASGLOBAL, &m_MainLoop->GetSceneManager ());
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction ("AudioSample@ GetSceneAudioStream ()",
            asMETHOD (SceneManager, GetSceneAudioStream), asCALL_THISCALL_ASGLOBAL, &m_MainLoop->GetSceneManager ());
        assert (r >= 0);
    }

    //--------------------------------------------------------------------------------------------------

    void ScriptManager::RegisterSceneAPI () {}

    //--------------------------------------------------------------------------------------------------

    void ScriptManager::RegisterCameraAPI ()
    {
        int r = m_ScriptEngine->RegisterObjectType ("Camera", sizeof (Camera), asOBJ_VALUE | asOBJ_POD);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalProperty ("Camera camera", &m_MainLoop->GetSceneManager ().GetCamera ());
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "Camera", "void SetTranslate (float dx, float dy)", asMETHOD (Camera, SetTranslate), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "Camera", "Point GetScale ()", asMETHOD (Camera, GetScale), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod (
            "Camera", "void SetCenter (float, float)", asMETHOD (Camera, SetCenter), asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("Camera",
            "void SetFollowActor (Actor@, Point followOffset = Point(0.f, 0.f))", asMETHOD (Camera, SetFollowActor),
            asCALL_THISCALL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterObjectMethod ("Camera",
            "void TweenToPoint (Point point, float timeMs = 1000, bool centerScreen = true)",
            asMETHODPR (Camera, TweenToPoint, (Point, float, bool), void), asCALL_THISCALL);
        assert (r >= 0);

        r = m_ScriptEngine->RegisterObjectMethod ("Camera",
            "void TweenToPoint (Point point, TweenFuncPointFinish @+ te, float timeMs = 1000, bool centerScreen = "
            "true)",
            asMETHODPR (Camera, TweenToPoint, (Point, asIScriptFunction*, float, bool), void), asCALL_THISCALL);
        assert (r >= 0);
    }

    //--------------------------------------------------------------------------------------------------

    void ScriptManager::RegisterGlobalAPI ()
    {
        int r = m_ScriptEngine->RegisterGlobalFunction (
            "void Log(const string &in)", asFUNCTIONPR (Log, (const std::string&), void), asCALL_CDECL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction (
            "void Log (float)", asFUNCTIONPR (Log, (float), void), asCALL_CDECL);
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction ("float GetDeltaTime ()", asMETHOD (Screen, GetDeltaTime),
            asCALL_THISCALL_ASGLOBAL, m_MainLoop->GetScreen ());
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction (
            "float GetFPS ()", asMETHOD (Screen, GetFPS), asCALL_THISCALL_ASGLOBAL, m_MainLoop->GetScreen ());
        assert (r >= 0);
        r = m_ScriptEngine->RegisterGlobalFunction ("void SetBackgroundColor (Color)",
            asMETHODPR (Screen, SetBackgroundColor, (ALLEGRO_COLOR), void), asCALL_THISCALL_ASGLOBAL,
            m_MainLoop->GetScreen ());
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
