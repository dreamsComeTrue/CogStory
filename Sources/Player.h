// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "Actor.h"

#define PLAYER_Z_ORDER 5

namespace aga
{
    class SceneManager;
    class Scene;

    class AudioSampleComponent;
    class ParticleEmitterComponent;

    struct ActorAction
    {
        Actor* AnActor = nullptr;
        asIScriptFunction* Func = nullptr;
        std::string SpeechID = "";
        bool Handled = false;
    };

    class Player : public Actor
    {
    public:
        static std::string TypeName;

    public:
        Player (SceneManager* sceneManager);
        virtual ~Player () override;
        bool Initialize () override;
        bool Destroy () override;

        void BeforeEnter () override;

        void SetPreventInput (bool prevent = false);
        bool IsPreventInput () const;
        void HandleInput (float deltaTime);
        bool ProcessEvent (ALLEGRO_EVENT* event, float deltaTime);

        bool IsAction ();

        bool Update (float deltaTime) override;
        void Render (float deltaTime) override;

        virtual void Move (float dx, float dy) override;
        virtual void SetPosition (float x, float y) override;
        virtual void SetPosition (Point point) override;

        virtual std::string GetTypeName () override;

        //  Override for ScriptManager
        void SetCurrentAnimation (const std::string& name) override;

        void SetActionHandler (asIScriptFunction* func);
        Actor* RegisterActorAction (
            const std::string& actionName, const std::string& actorName, asIScriptFunction* func);
        void RegisterActionSpeech (const std::string& actorName, const std::string& speechID);
        class SpeechFrame* TalkTo (Actor* actor, const std::string& speechID);
        Actor* GetLastActionActor ();

    private:
        void CreateParticleEmitters ();
        void UpdateParticleEmitters ();

        void HandleAction ();

    private:
        bool m_PreventInput;
        bool m_ActionHandling;

        AudioSampleComponent* m_FootStepComponent;
        ParticleEmitterComponent* m_HeadParticleComponent;
        ParticleEmitterComponent* m_FootParticleComponent;

        asIScriptFunction* m_ActionHandler;

        std::map<std::string, ActorAction> m_ActorActions;
        Actor* m_LastActionActor;
    };
}

#endif //   __PLAYER_H__
