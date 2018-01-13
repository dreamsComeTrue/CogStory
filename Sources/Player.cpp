// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.
#include "Player.h"
#include "AudioSample.h"
#include "MainLoop.h"
#include "PhysicsManager.h"
#include "Scene.h"
#include "SceneManager.h"
#include "Screen.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    std::string Player::TypeName = "Player";

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    Player::Player (SceneManager* sceneManager)
      : Actor (sceneManager)
      , m_FollowCamera (true)
      , m_PreventInput (false)
    {
    }

    //--------------------------------------------------------------------------------------------------

    Player::~Player ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    AudioSample* sample;

    bool Player::Initialize ()
    {
        Actor::Initialize ();

        m_Image = al_load_bitmap (GetResourcePath (ResourceID::GFX_PLAYER).c_str ());
        Bounds.SetSize ({ 64, 64 });

        InitializeAnimations ();

        sample = m_SceneManager->GetMainLoop ()->GetAudioManager ().LoadSampleFromFile (
          "FOOT_STEP", GetResourcePath (SOUND_FOOT_STEP));
        sample->SetVolume (1.0f);

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void Player::BeforeEnter ()
    {
        PhysPoints.clear ();
        PhysPoints.push_back ({ { 20, 30 }, { 25, 20 }, { 39, 20 }, { 44, 30 }, { 44, 64 }, { 20, 64 } });
        SetPhysOffset (Bounds.GetPos ().X - Bounds.GetHalfSize ().Width,
                       Bounds.GetPos ().Y - Bounds.GetHalfSize ().Height);
    }

    //--------------------------------------------------------------------------------------------------

    void Player::InitializeAnimations ()
    {
        Point cellSize (64, 64);
        AnimationFrames idleFrames (3, cellSize);
        idleFrames.AddFrame (0, 0, 0);
        idleFrames.AddFrame (1, 0, 1);
        idleFrames.AddFrame (2, 0, 2);
        idleFrames.SetPlaySpeed (500);
        m_Animation.AddAnimationFrames (ANIM_IDLE_NAME, idleFrames);

        AnimationFrames moveLeftFrames (1, cellSize);
        moveLeftFrames.AddFrame (0, 1, 0);
        moveLeftFrames.SetPlaySpeed (500);
        m_Animation.AddAnimationFrames (ANIM_MOVE_LEFT_NAME, moveLeftFrames);

        AnimationFrames moveRightFrames (1, cellSize);
        moveRightFrames.AddFrame (0, 2, 0);
        moveRightFrames.SetPlaySpeed (500);
        m_Animation.AddAnimationFrames (ANIM_MOVE_RIGHT_NAME, moveRightFrames);

        AnimationFrames moveUpFrames (1, cellSize);
        moveUpFrames.AddFrame (0, 3, 0);
        moveUpFrames.SetPlaySpeed (500);
        m_Animation.AddAnimationFrames (ANIM_MOVE_UP_NAME, moveUpFrames);

        SetCurrentAnimation (ANIM_IDLE_NAME);
    }

    //--------------------------------------------------------------------------------------------------

    void Player::ProcessEvent (ALLEGRO_EVENT* event, float deltaTime)
    {
        if (event->type == ALLEGRO_EVENT_KEY_UP)
        {
            SetCurrentAnimation (ANIM_IDLE_NAME);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Player::SetPreventInput (bool prevent) { m_PreventInput = prevent; }

    //--------------------------------------------------------------------------------------------------

    bool Player::IsPreventInput () const { return m_PreventInput; }

    //--------------------------------------------------------------------------------------------------

    void Player::HandleInput (float deltaTime)
    {
        if (m_PreventInput)
        {
            return;
        }

        ALLEGRO_KEYBOARD_STATE state;
        al_get_keyboard_state (&state);
        float dx = 0, dy = 0;

        if (al_key_down (&state, ALLEGRO_KEY_DOWN) || al_key_down (&state, ALLEGRO_KEY_S))
        {
            dy = MOVE_SPEED * deltaTime;
            SetCurrentAnimation (ANIM_IDLE_NAME);
        }

        if (al_key_down (&state, ALLEGRO_KEY_UP) || al_key_down (&state, ALLEGRO_KEY_W))
        {
            dy = -MOVE_SPEED * deltaTime;
            SetCurrentAnimation (ANIM_MOVE_UP_NAME);
        }

        if (al_key_down (&state, ALLEGRO_KEY_RIGHT) || al_key_down (&state, ALLEGRO_KEY_D))
        {
            dx = MOVE_SPEED * deltaTime;
            SetCurrentAnimation (ANIM_MOVE_RIGHT_NAME);
        }

        if (al_key_down (&state, ALLEGRO_KEY_LEFT) || al_key_down (&state, ALLEGRO_KEY_A))
        {
            dx = -MOVE_SPEED * deltaTime;
            SetCurrentAnimation (ANIM_MOVE_LEFT_NAME);
        }

        std::map<std::string, TriggerArea>& triggerAreas = m_SceneManager->GetActiveScene ()->GetTriggerAreas ();

        for (std::map<std::string, TriggerArea>::iterator it = triggerAreas.begin (); it != triggerAreas.end (); ++it)
        {
            TriggerArea& area = it->second;

            for (Polygon& polygon : area.Polygons)
            {
                if (area.OnEnterCallback || area.ScriptOnEnterCallback || area.OnLeaveCallback ||
                    area.ScriptOnLeaveCallback)
                {
                    PolygonCollisionResult r = m_SceneManager->GetMainLoop ()->GetPhysicsManager ().PolygonCollision (
                      GetPhysPolygon (0), polygon, { dx, dy });

                    if (r.WillIntersect || r.Intersect)
                    {
                        if (!area.WasEntered)
                        {
                            if (area.OnEnterCallback)
                            {
                                area.OnEnterCallback (dx + r.MinimumTranslationVector.X,
                                                      dy + r.MinimumTranslationVector.Y);
                            }

                            if (area.ScriptOnEnterCallback)
                            {
                                const char* moduleName = area.ScriptOnEnterCallback->GetModuleName ();
                                Script* script =
                                  m_SceneManager->GetMainLoop ()->GetScriptManager ().GetScriptByModuleName (
                                    moduleName);

                                if (script)
                                {
                                    Point point = { dx + r.MinimumTranslationVector.X,
                                                    dy + r.MinimumTranslationVector.Y };
                                    asIScriptContext* ctx = script->GetContext ();
                                    ctx->Prepare (area.ScriptOnEnterCallback);
                                    ctx->SetArgObject (0, &point);

                                    ctx->Execute ();
                                }
                            }
                        }

                        area.WasEntered = true;
                    }
                    else if (area.WasEntered)
                    {
                        area.WasEntered = false;

                        if (area.OnLeaveCallback)
                        {
                            area.OnLeaveCallback (dx + r.MinimumTranslationVector.X, dy + r.MinimumTranslationVector.Y);
                        }

                        if (area.ScriptOnLeaveCallback)
                        {
                            const char* moduleName = area.ScriptOnLeaveCallback->GetModuleName ();
                            Script* script =
                              m_SceneManager->GetMainLoop ()->GetScriptManager ().GetScriptByModuleName (moduleName);

                            if (script)
                            {
                                Point point = { dx + r.MinimumTranslationVector.X, dy + r.MinimumTranslationVector.Y };
                                asIScriptContext* ctx = script->GetContext ();
                                ctx->Prepare (area.ScriptOnLeaveCallback);
                                ctx->SetArgObject (0, &point);

                                ctx->Execute ();
                            }
                        }
                    }
                }
            }
        }

        std::vector<Entity*> entites = m_SceneManager->GetActiveScene ()->GetVisibleEntities ();
        for (Entity* ent : entites)
        {
            Tile* tile = (Tile*)ent;

            if (!tile->PhysPoints.empty ())
            {
                for (int i = 0; i < tile->GetPhysPolygonsCount (); ++i)
                {
                    PolygonCollisionResult r = m_SceneManager->GetMainLoop ()->GetPhysicsManager ().PolygonCollision (
                      GetPhysPolygon (0), tile->GetPhysPolygon (i), { dx, dy });

                    if (r.WillIntersect)
                    {
                        dx = dx + r.MinimumTranslationVector.X;
                        dy = dy + r.MinimumTranslationVector.Y;
                    }
                }
            }
        }

        if (!(AreSame (dx, 0) && AreSame (dy, 0)))
        {
            if (al_key_down (&state, ALLEGRO_KEY_LSHIFT))
            {
                float multiplier = 3;
                Move (dx * multiplier, dy * multiplier);
            }
            else
            {
                Move (dx, dy);
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool Player::Update (float deltaTime) { Actor::Update (deltaTime); }

    //--------------------------------------------------------------------------------------------------

    float sampleCounter = 0;

    void Player::Move (float dx, float dy)
    {
        sampleCounter += m_SceneManager->GetMainLoop ()->GetScreen ()->GetDeltaTime ();

        if (sampleCounter > 0.4f)
        {
            sample->Play ();
            sampleCounter = 0;
        }

        m_OldPosition = Bounds.GetPos ();
        Bounds.SetPos (Bounds.GetPos () + Point (dx, dy));

        if (!(std::abs (dx) < 0.1 && std::abs (dy) < 0.1f))
        {
            ChooseAnimation (ToPositiveAngle (RadiansToDegrees (std::atan2 (dy, dx))));
        }

        if (MoveCallback != nullptr)
        {
            MoveCallback (dx, dy);
        }

        AddPhysOffset ({ dx, dy });
    }

    //--------------------------------------------------------------------------------------------------

    void Player::SetPosition (float x, float y)
    {
        m_OldPosition = Bounds.GetPos ();
        Bounds.SetPos ({ x, y });

        if (MoveCallback != nullptr && m_FollowCamera)
        {
            MoveCallback (Bounds.GetPos ().X - m_OldPosition.X, Bounds.GetPos ().Y - m_OldPosition.Y);
        }

        SetPhysOffset (Bounds.GetPos ().X - Bounds.GetHalfSize ().Width,
                       Bounds.GetPos ().Y - Bounds.GetHalfSize ().Height);
    }

    //--------------------------------------------------------------------------------------------------

    void Player::SetFollowCamera (bool follow) { m_FollowCamera = follow; }

    //--------------------------------------------------------------------------------------------------

    std::string Player::GetTypeName () { return TypeName; }

    //--------------------------------------------------------------------------------------------------
}
