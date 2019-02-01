// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __SCRIPT_MANAGER_H__
#define __SCRIPT_MANAGER_H__

#include "Common.h"

namespace aga
{
    class MainLoop;
    class Script;

    class FileUpdateListener : public FW::FileWatchListener
    {
    public:
        FileUpdateListener (class SceneManager* sceneManager)
            : m_SceneManager (sceneManager)
        {
        }

        void handleFileAction (
            FW::WatchID watchid, const FW::String& dir, const FW::String& filename, FW::Action action) override;

    private:
        class SceneManager* m_SceneManager;
    };

    class ScriptManager : public Lifecycle
    {
    public:
        ScriptManager (MainLoop* mainLoop);
        virtual ~ScriptManager ();
        bool Initialize ();
        bool Destroy ();

        void Update (float deltaTime);

        Script* LoadScriptFromFile (const std::string& path, const std::string& moduleName);
        Script* LoadScriptFromText (const std::string& text, const std::string& moduleName);
        asIScriptEngine* GetEngine ();
        asIScriptContext* GetContext ();

        void RemoveScript (const std::string& name);
        void RemoveScript (Script* script);

        void RunScriptFunction (asIScriptFunction* func, void* obj);
        void RunScriptFunction (asIScriptFunction* func, void* obj1, void* obj2);

        Script* GetScriptByModuleName (const std::string& moduleName);

    private:
        void RegisterAPI ();

        void RegisterEnums ();
        void RegisterPointAPI ();
        void RegisterColorAPI ();
        void RegisterRectAPI ();
        void RegisterTweenAPI ();
        void RegisterTimelineAPI ();
        void RegisterFlagPointAPI ();
        void RegisterPlayerAPI ();
        void RegisterScreenAPI ();
        void RegisterAudioSampleAPI ();
        void RegisterAudioStreamAPI ();
        void RegisterAudioManagerAPI ();
        void RegisterTriggerAreaAPI ();
        void RegisterSpeechFrameAPI ();
        void RegisterSpeechFrameManagerAPI ();
        void RegisterParticleEmitterAPI ();
        void RegisterActorAPI ();
        void RegisterMovementComponentAPI ();
        void RegisterParticleEmitterComponentAPI ();
        void RegisterSceneManagerAPI ();
        void RegisterSceneAPI ();
        void RegisterCameraAPI ();
        void RegisterScriptAPI ();
        void RegisterGlobalAPI ();

        template <class T> void RegisterBaseActorAPI (const char* typeName);
        template <class T> void RegisterDerivedPlayerAPI (const char* typeName);

    private:
        MainLoop* m_MainLoop;
        asIScriptEngine* m_ScriptEngine;
        std::map<std::string, Script*> m_Scripts;

        FW::FileWatcher m_FileWatcher;
        FileUpdateListener m_FileListener;
    };

    extern char g_ScriptErrorBuffer[1024];
}

#endif //   __SCRIPT_MANAGER_H__
