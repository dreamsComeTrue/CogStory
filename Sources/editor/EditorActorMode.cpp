// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorActorMode.h"
#include "ActorFactory.h"
#include "Editor.h"
#include "MainLoop.h"
#include "SceneManager.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    EditorActorMode::EditorActorMode (Editor* editor)
        : m_Editor (editor)
        , m_SelectedActor (nullptr)
        , m_ActorUnderCursor (nullptr)
        , m_Rotation (0)
    {
    }

    //--------------------------------------------------------------------------------------------------

    EditorActorMode::~EditorActorMode () {}

    //--------------------------------------------------------------------------------------------------

    bool EditorActorMode::AddOrUpdateActor (const std::string& name, const std::string& actorType, Point pos,
                                            float rotation)
    {
        Actor* actor = m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->GetActor (name);

        if (actor)
        {
            actor->Name = name;
            actor->Bounds.Pos = pos;
            actor->TemplateBounds.Pos = pos;
            actor->Rotation = rotation;
        }
        else
        {
            Actor* newActor = ActorFactory::GetActor (&m_Editor->m_MainLoop->GetSceneManager (), actorType);
            newActor->Name = name;
            newActor->Bounds.Pos = pos;
            newActor->TemplateBounds.Pos = pos;
            newActor->Rotation = rotation;

            m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->AddActor (name, newActor);
        }

        m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->SortActors ();

        m_Editor->m_EditorActorMode.Clear ();

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::RemoveActor (const std::string& name)
    {
        m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->RemoveActor (name);
        m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->SortActors ();
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::Clear () {}

    //--------------------------------------------------------------------------------------------------

    bool EditorActorMode::MoveSelectedActor ()
    {
        ALLEGRO_MOUSE_STATE state;
        al_get_mouse_state (&state);

        if (state.buttons == 1 && m_SelectedActor)
        {
            Point translate = m_Editor->m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
            Point scale = m_Editor->m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();
            Point point = m_Editor->CalculateCursorPoint (state.x + m_TileSelectionOffset.X, state.y + m_TileSelectionOffset.Y);

            m_SelectedActor->Bounds.SetPos (
                { (translate.X + point.X) * 1 / scale.X, (translate.Y + point.Y) * 1 / scale.Y });
            m_SelectedActor->TemplateBounds = m_SelectedActor->Bounds;
            m_SelectedActor->SetPhysOffset (
                m_SelectedActor->Bounds.GetPos ().X - m_SelectedActor->Bounds.GetHalfSize ().Width,
                m_SelectedActor->Bounds.GetPos ().Y - m_SelectedActor->Bounds.GetHalfSize ().Height);

            QuadTreeNode& quadTree = m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->GetQuadTree ();
            quadTree.Remove (m_SelectedActor);
            quadTree.UpdateStructures ();
            quadTree.Insert (m_SelectedActor);
            quadTree.UpdateStructures ();
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    Actor* EditorActorMode::GetActorUnderCursor (int mouseX, int mouseY, Rect&& outRect)
    {
        std::vector<Actor*>& actors = m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->GetActors ();
        Actor* result = nullptr;

        for (Actor* actorIt : actors)
        {
            Rect r = m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->GetRenderBounds (actorIt);

            if (InsideRect (mouseX, mouseY, r))
            {
                if ((result == nullptr) || (result && (result->RenderID < actorIt->RenderID)))
                {
                    outRect = r;
                    result = actorIt;

                    m_TileSelectionOffset = { r.GetTopLeft ().X - mouseX + r.GetSize ().Width * 0.5f,
                                              r.GetTopLeft ().Y - mouseY + r.GetSize ().Height * 0.5f };
                }
            }
        }

        return result;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::ChangeRotation (bool clockwise)
    {
        m_Rotation += clockwise ? -15 : 15;

        if (m_Rotation <= -360)
        {
            m_Rotation = 0;
        }

        if (m_Rotation >= 360)
        {
            m_Rotation = 0;
        }

        if (m_SelectedActor)
        {
            m_SelectedActor->Rotation = m_Rotation;
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::ChangeZOrder (bool clockwise)
    {
        if (m_SelectedActor)
        {
            m_SelectedActor->ZOrder += clockwise ? -1 : 1;

            int currentID = m_SelectedActor->ID;

            m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->SortActors ();

            std::vector<Actor*>& actors = m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->GetActors ();

            for (Actor* actor : actors)
            {
                if (actor->ID == currentID)
                {
                    m_SelectedActor = actor;
                    break;
                }
            }
        }
    }

    //--------------------------------------------------------------------------------------------------
}
