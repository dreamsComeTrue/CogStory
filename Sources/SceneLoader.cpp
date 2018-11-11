// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "SceneLoader.h"
#include "ActorFactory.h"
#include "Atlas.h"
#include "Component.h"
#include "MainLoop.h"
#include "Scene.h"

using json = nlohmann::json;

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    /*
     *
     *  {
     *      "name": "Home",
     *      "flag_points" :
     *      [
     *          "SPAWN_1" : "0 0"
     *      ]
     *  }
     *
     */

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    Scene* SceneLoader::LoadScene (SceneManager* sceneManager, const std::string& filePath, bool loadBounds)
    {
        try
        {
            std::ifstream file ((GetDataPath () + "scenes/" + filePath).c_str ());
            json j;
            file >> j;
            file.close ();

            Rect size;

            if (loadBounds)
            {
                size = Rect (StringToPoint (j["min_size"]), StringToPoint (j["max_size"]));
            }
            else
            {
                size = Rect ({-SCENE_INFINITE_BOUND_SIZE, -SCENE_INFINITE_BOUND_SIZE},
                    {SCENE_INFINITE_BOUND_SIZE, SCENE_INFINITE_BOUND_SIZE});
            }

            Scene* scene = new Scene (sceneManager, size);
            scene->m_Size = size;
            scene->m_Name = j["name"];
            scene->m_Path = filePath;
            scene->m_PlayerStartLocation = StringToPoint (j["player_start"]);

            std::vector<int> ints = StringToInts (j["color"]);
            scene->m_BackgroundColor
                = al_map_rgba (static_cast<unsigned char> (ints[0]), static_cast<unsigned char> (ints[1]),
                    static_cast<unsigned char> (ints[2]), static_cast<unsigned char> (ints[3]));

            auto& flag_points = j["flag_points"];

            for (auto& flag_point : flag_points)
            {
                std::string name = flag_point["name"];
                Point pos = StringToPoint (flag_point["pos"]);

                scene->AddFlagPoint (name, pos);
            }

            for (std::map<std::string, FlagPoint>::iterator it = scene->m_FlagPoints.begin ();
                 it != scene->m_FlagPoints.end (); ++it)
            {
                for (auto& flag_point : flag_points)
                {
                    std::string name = flag_point["name"];

                    if (name == it->first)
                    {
                        std::vector<std::string> connections = StringToVectorStrings (flag_point["connections"]);

                        if (!connections.empty ())
                        {
                            for (std::map<std::string, FlagPoint>::iterator it2 = scene->m_FlagPoints.begin ();
                                 it2 != scene->m_FlagPoints.end (); ++it2)
                            {
                                for (size_t i = 0; i < connections.size (); ++i)
                                {
                                    if (connections[i] == it2->first)
                                    {
                                        it->second.Connections.push_back (&it2->second);
                                        break;
                                    }
                                }
                            }

                            break;
                        }
                    }
                }
            }

            auto& triggerAreas = j["trigger_areas"];

            for (auto& triggerArea : triggerAreas)
            {
                std::string name = triggerArea["name"];
                std::string data;

                if (!triggerArea["data"].is_null () && triggerArea["data"] != "")
                {
                    data = triggerArea["data"];
                }

                std::vector<Point> poly = StringToVectorPoints (triggerArea["poly"]);
                std::string collidableStr = triggerArea["collidable"];

                scene->AddTriggerArea (name, data, poly, atoi (collidableStr.c_str ()));

                if (data != "")
                {
                    scene->AddSceneTransition (name, data);
                    scene->AddOnEnterCallback (name, [=](std::string areaName, float, float) {
                        sceneManager->SetActiveScene (scene->GetSceneTransition (areaName), true);

                        return false;
                    });
                }
            }

            auto& actors = j["actors"];

            for (auto& actorIt : actors)
            {
                Actor* newActor = ActorFactory::GetActor (sceneManager, actorIt["type"]);

                if (newActor)
                {
                    std::string id = actorIt["id"];
                    newActor->ID = atoi (id.c_str ());

                    std::string blueprintID = actorIt["blueprint"];
                    newActor->BlueprintID = atoi (blueprintID.c_str ());
                    newActor->Name = actorIt["name"];
                    newActor->Bounds.Pos = StringToPoint (actorIt["pos"]);
                    newActor->Bounds.Size = StringToPoint (actorIt["size"]);
                    newActor->TemplateBounds = newActor->Bounds;

                    std::string zOrder = actorIt["z-order"];
                    newActor->ZOrder = atoi (zOrder.c_str ());
                    std::string rot = actorIt["rot"];
                    newActor->Rotation = static_cast<float> (atof (rot.c_str ()));
                    std::string focusHeight = actorIt["focus-height"];
                    newActor->SetFocusHeight (static_cast<float> (atof (focusHeight.c_str ())));

                    if (!actorIt["atlas"].is_null ())
                    {
                        Atlas* atlas = sceneManager->GetMainLoop ()->GetAtlasManager ().GetAtlas (actorIt["atlas"]);

                        newActor->SetAtlas (atlas);
                        newActor->SetAtlasName (actorIt["atlas"]);
                        newActor->SetAtlasRegionName (actorIt["atlas-region"]);
                    }

                    if (!actorIt["animation"].is_null ())
                    {
                        Animation& animation = ActorFactory::GetAnimation (actorIt["animation"]);
                        newActor->SetAnimation (animation);
                    }

                    //  Physics
                    if (!actorIt["phys"].is_null ())
                    {
                        auto& physTiles = actorIt["phys"];

                        for (auto& physTile : physTiles)
                        {
                            newActor->PhysPoints.push_back (StringToVectorPoints (physTile["poly"]));
                        }

                        newActor->SetPhysOffset (newActor->Bounds.GetPos () + newActor->Bounds.GetHalfSize ());
                    }

                    auto& scripts = actorIt["scripts"];

                    for (auto& scriptIt : scripts)
                    {
                        std::string name = scriptIt["name"];
                        std::string path = scriptIt["path"];

                        newActor->AttachScript (name, path);
                    }

                    auto& components = actorIt["components"];

                    for (auto& component : components)
                    {
                        std::string name = component["name"];
                        std::string type = component["type"];

                        Component* componentObj = ActorFactory::GetActorComponent (newActor, type);

                        if (componentObj)
                        {
                            newActor->AddComponent (name, componentObj);
                        }
                    }

                    std::string collisionStr = actorIt["collision"];
                    newActor->SetCollisionEnabled (atoi (collisionStr.c_str ()));

                    std::string collidableStr = actorIt["collidable"];
                    newActor->SetCollidable (atoi (collidableStr.c_str ()));

                    std::string overlapStr = actorIt["overlap"];
                    newActor->SetCheckOverlap (atoi (overlapStr.c_str ()));

                    newActor->Initialize ();

                    scene->AddActor (newActor);
                }
            }

            UpdateMaxEntityID (scene);
            scene->SortActors ();

            auto& speeches = j["speeches"];

            for (auto& speech : speeches)
            {
                SpeechData speechData;
                speechData.ID = speech["id"];
                speechData.Name = speech["name"];
                speechData.Group = speech["group"];
                speechData.Action = speech["action"];
                speechData.ActorRegionName = speech["region_name"];
                speechData.MaxCharsInLine = speech["max_chars"];
                speechData.MaxLines = speech["max_lines"];
                speechData.AbsoluteFramePosition = StringToPoint (speech["abs_pos"]);
                speechData.RelativeFramePosition = speech["rel_pos"];

                auto& texts = speech["texts"];

                for (auto& text : texts)
                {
                    if (text["langID"] == "EN")
                    {
                        speechData.Text[LANG_EN] = text["data"];
                    }

                    if (text["langID"] == "PL")
                    {
                        speechData.Text[LANG_PL] = text["data"];
                    }
                }

                auto& outcomes = speech["outcomes"];

                for (auto& outcome : outcomes)
                {
                    auto& texts = outcome["texts"];

                    for (auto& text : texts)
                    {
                        SpeechOutcome out;
                        out.Name = text["name"];
                        out.Text = text["data"];
                        out.Action = text["action"];

                        if (outcome["langID"] == "EN")
                        {
                            speechData.Outcomes[LANG_EN].push_back (out);
                        }
                        else if (outcome["langID"] == "PL")
                        {
                            speechData.Outcomes[LANG_PL].push_back (out);
                        }
                    }
                }

                scene->AddSpeech (speechData);
            }

            auto& scripts = j["scripts"];

            for (auto& j_tile : scripts)
            {
                std::string name = j_tile["name"];
                std::string path = j_tile["path"];

                scene->AttachScript (name, path);
            }

            return scene;
        }
        catch (const std::exception&)
        {
            return nullptr;
        }
    }

    //--------------------------------------------------------------------------------------------------

    void SceneLoader::SaveScene (Scene* scene, const std::string& filePath)
    {
        Point minRect {std::numeric_limits<float>::max (), std::numeric_limits<float>::max ()};
        Point maxRect {std::numeric_limits<float>::min (), std::numeric_limits<float>::min ()};

        try
        {
            json j;

            j["name"] = scene->m_Name;

            ALLEGRO_COLOR color = scene->m_BackgroundColor;
            j["color"] = IntsToString ({static_cast<int> (color.r * 255.f), static_cast<int> (color.g * 255.f),
                static_cast<int> (color.b * 255.f), static_cast<int> (color.a * 255.f)});

            j["player_start"] = PointToString (scene->GetPlayerStartLocation ());

            j["scripts"] = json::array ({});

            for (ScriptMetaData& script : scene->m_Scripts)
            {
                json scriptObj = json::object ({});

                scriptObj["name"] = script.Name;
                scriptObj["path"] = script.Path;

                j["scripts"].push_back (scriptObj);
            }

            j["actors"] = json::array ({});

            for (Actor* actor : scene->m_Actors)
            {
                if (actor->Bounds.GetTopLeft ().X < minRect.X)
                {
                    minRect.X = actor->Bounds.GetTopLeft ().X;
                }
                if (actor->Bounds.GetTopLeft ().Y < minRect.Y)
                {
                    minRect.Y = actor->Bounds.GetTopLeft ().Y;
                }
                if (actor->Bounds.GetBottomRight ().X > maxRect.X)
                {
                    maxRect.X = actor->Bounds.GetBottomRight ().X;
                }
                if (actor->Bounds.GetBottomRight ().Y > maxRect.Y)
                {
                    maxRect.Y = actor->Bounds.GetBottomRight ().Y;
                }

                json actorObj = json::object ({});

                std::stringstream streamRotation;
                streamRotation << std::fixed << std::setprecision (2) << actor->Rotation;

                std::stringstream streamFucsHeight;
                streamFucsHeight << std::fixed << std::setprecision (2) << actor->GetFocusHeight ();

                actorObj["id"] = std::to_string (actor->ID);
                actorObj["type"] = actor->GetTypeName ();
                actorObj["name"] = actor->Name;
                actorObj["blueprint"] = std::to_string (actor->BlueprintID);
                actorObj["pos"] = PointToString (actor->Bounds.GetPos ());
                actorObj["size"] = PointToString (actor->Bounds.GetSize ());
                actorObj["z-order"] = std::to_string (actor->ZOrder);
                actorObj["rot"] = streamRotation.str ();
                actorObj["atlas"] = actor != nullptr ? actor->GetAtlas ()->GetName () : "";
                actorObj["atlas-region"] = actor->GetAtlasRegionName ();
                actorObj["animation"] = actor->GetAnimation ().GetName ();
                actorObj["collision"] = std::to_string (actor->IsCollisionEnabled ());
                actorObj["collidable"] = std::to_string (actor->IsCollidable ());
                actorObj["overlap"] = std::to_string (actor->IsCheckOverlap ());
                actorObj["focus-height"] = streamFucsHeight.str ();

                actorObj["phys"] = json::array ({});

                for (size_t i = 0; i < actor->PhysPoints.size (); ++i)
                {
                    std::vector<Point>& points = actor->PhysPoints[i];

                    if (!points.empty ())
                    {
                        json physObj = json::object ({});

                        physObj["poly"] = VectorPointsToString (points);

                        actorObj["phys"].push_back (physObj);
                    }
                }

                actorObj["scripts"] = json::array ({});

                for (ScriptMetaData& script : actor->GetScripts ())
                {
                    json scriptObj = json::object ({});

                    scriptObj["name"] = script.Name;
                    scriptObj["path"] = script.Path;

                    actorObj["scripts"].push_back (scriptObj);
                }

                actorObj["components"] = json::array ({});

                std::map<std::string, Component*>& components = actor->GetComponents ();
                for (std::map<std::string, Component*>::iterator it = components.begin (); it != components.end ();
                     ++it)
                {
                    json componentObj = json::object ({});

                    componentObj["name"] = it->first;
                    componentObj["type"] = it->second->GetTypeName ();

                    actorObj["components"].push_back (componentObj);
                }

                j["actors"].push_back (actorObj);
            }

            j["min_size"] = PointToString (minRect);
            j["max_size"] = PointToString (maxRect);

            j["flag_points"] = json::array ({});

            for (std::map<std::string, FlagPoint>::iterator it = scene->m_FlagPoints.begin ();
                 it != scene->m_FlagPoints.end (); ++it)
            {
                json flagObj = json::object ({});

                flagObj["name"] = it->first;
                flagObj["pos"] = PointToString (it->second.Pos);

                std::vector<std::string> connections;
                for (size_t i = 0; i < it->second.Connections.size (); ++i)
                {
                    connections.push_back (it->second.Connections[i]->Name);
                }

                flagObj["connections"] = VectorStringsToString (connections);

                j["flag_points"].push_back (flagObj);
            }

            j["trigger_areas"] = json::array ({});

            for (std::map<std::string, TriggerArea>::iterator it = scene->m_TriggerAreas.begin ();
                 it != scene->m_TriggerAreas.end (); ++it)
            {
                json triggerObj = json::object ({});

                triggerObj["name"] = it->second.Name;
                triggerObj["data"] = it->second.Data;
                triggerObj["collidable"] = std::to_string (it->second.Collidable);
                triggerObj["poly"] = VectorPointsToString (it->second.Points);

                j["trigger_areas"].push_back (triggerObj);
            }

            j["speeches"] = json::array ({});

            for (std::map<long, SpeechData>::iterator it = scene->m_Speeches.begin (); it != scene->m_Speeches.end ();
                 ++it)
            {
                json speechObj = json::object ({});

                speechObj["id"] = it->first;
                speechObj["name"] = it->second.Name;
                speechObj["group"] = it->second.Group;
                speechObj["action"] = it->second.Action;
                speechObj["region_name"] = it->second.ActorRegionName;
                speechObj["max_chars"] = it->second.MaxCharsInLine;
                speechObj["max_lines"] = it->second.MaxLines;
                speechObj["abs_pos"] = PointToString (it->second.AbsoluteFramePosition);
                speechObj["rel_pos"] = static_cast<int> (it->second.RelativeFramePosition);

                speechObj["texts"] = json::array ({});

                json textObj = json::object ({});
                textObj["langID"] = "EN";
                textObj["data"] = it->second.Text[LANG_EN];

                speechObj["texts"].push_back (textObj);

                textObj["langID"] = "PL";
                textObj["data"] = it->second.Text[LANG_PL];

                speechObj["texts"].push_back (textObj);

                speechObj["outcomes"] = json::array ({});

                //  EN - Outcome
                json outcomeObj = json::object ({});
                outcomeObj["langID"] = "EN";

                outcomeObj["texts"] = json::array ({});

                for (size_t i = 0; i < it->second.Outcomes[LANG_EN].size (); ++i)
                {
                    json outComeTextObj = json::object ({});
                    outComeTextObj["name"] = it->second.Outcomes[LANG_EN][i].Name;
                    outComeTextObj["data"] = it->second.Outcomes[LANG_EN][i].Text;
                    outComeTextObj["action"] = it->second.Outcomes[LANG_EN][i].Action;

                    outcomeObj["texts"].push_back (outComeTextObj);
                }

                speechObj["outcomes"].push_back (outcomeObj);

                //  PL - Outcome
                outcomeObj = json::object ({});
                outcomeObj["langID"] = "PL";

                outcomeObj["texts"] = json::array ({});

                for (size_t i = 0; i < it->second.Outcomes[LANG_PL].size (); ++i)
                {
                    json outComeTextObj = json::object ({});
                    outComeTextObj["name"] = it->second.Outcomes[LANG_PL][i].Name;
                    outComeTextObj["data"] = it->second.Outcomes[LANG_PL][i].Text;
                    outComeTextObj["action"] = it->second.Outcomes[LANG_PL][i].Action;

                    outcomeObj["texts"].push_back (outComeTextObj);
                }

                speechObj["outcomes"].push_back (outcomeObj);

                j["speeches"].push_back (speechObj);
            }

            // write prettified JSON to another file
            std::ofstream out (filePath);
            out << std::setw (4) << j.dump (4, ' ') << "\n";
        }
        catch (const std::exception&)
        {
        }
    }

    //--------------------------------------------------------------------------------------------------

    void SceneLoader::UpdateMaxEntityID (Scene* scene)
    {
        int maxEntID = -1;

        std::vector<Actor*>& actors = scene->GetActors ();

        for (Actor* act : actors)
        {
            if (act->ID > maxEntID)
            {
                maxEntID = act->ID + 1;
            }
        }

        Entity::GlobalID = maxEntID;
    }

    //--------------------------------------------------------------------------------------------------
}
