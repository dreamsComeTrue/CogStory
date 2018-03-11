// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "SceneLoader.h"
#include "ActorFactory.h"
#include "Atlas.h"
#include "AtlasManager.h"
#include "MainLoop.h"
#include "SceneManager.h"
#include "actors/TileActor.h"

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

    auto StringToVectorPoints = [](std::string in) -> std::vector<Point> {
        size_t count = 0;
        const char* delimiter = " ";
        std::vector<Point> nums;
        char* str = const_cast<char*> (in.c_str ());

        for (char* pch = strtok (str, delimiter); pch != NULL; pch = strtok (NULL, delimiter))
        {
            float x = atof (pch);

            pch = strtok (NULL, delimiter);

            float y = atof (pch);

            nums.push_back ({ x, y });
        }

        return nums;
    };

    //--------------------------------------------------------------------------------------------------

    auto StringToVectorStrings = [](std::string in) -> std::vector<std::string> {
        size_t count = 0;
        const char* delimiter = " ";
        std::vector<std::string> strings;
        char* str = const_cast<char*> (in.c_str ());

        for (char* pch = strtok (str, delimiter); pch != NULL; pch = strtok (NULL, delimiter))
        {
            strings.push_back (pch);
        }

        return strings;
    };

    //--------------------------------------------------------------------------------------------------

    auto StringToPoint = [](std::string in) -> Point {
        std::vector<Point> vec = StringToVectorPoints (in);

        return vec[0];
    };

    //--------------------------------------------------------------------------------------------------

    auto PointToString = [](Point in) -> std::string { return ToString (in.X) + " " + ToString (in.Y); };

    //--------------------------------------------------------------------------------------------------

    auto IntsToString = [](std::vector<int> data) -> std::string {
        std::string result;

        for (int i = 0; i < data.size (); ++i)
        {
            result += ToString (data[i]);

            if (i < data.size () - 1)
            {
                result += " ";
            }
        }

        return result;
    };

    //--------------------------------------------------------------------------------------------------

    auto StringToInts = [](std::string in) -> std::vector<int> {
        size_t count = 0;
        const char* delimiter = " ";
        std::vector<int> ints;
        char* str = const_cast<char*> (in.c_str ());

        for (char* pch = strtok (str, delimiter); pch != NULL; pch = strtok (NULL, delimiter))
        {
            ints.push_back (atoi (pch));
        }

        return ints;
    };

    //--------------------------------------------------------------------------------------------------

    auto VectorPointsToString = [](std::vector<Point>& points) -> std::string {
        std::string out;

        for (Point& p : points)
        {
            out += PointToString (p) + " ";
        }

        return out;
    };

    //--------------------------------------------------------------------------------------------------

    auto VectorStringsToString = [](std::vector<std::string>& strings) -> std::string {
        std::string out;

        for (std::string& s : strings)
        {
            out += s + " ";
        }

        return out;
    };

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    const float boundSize = 10000;

    //--------------------------------------------------------------------------------------------------

    Scene* SceneLoader::LoadScene (SceneManager* sceneManager, const std::string& filePath)
    {
        try
        {
            Scene* scene = new Scene (sceneManager);
            std::ifstream file ((GetDataPath () + "scenes/" + filePath).c_str ());
            json j;
            file >> j;
            file.close ();

            scene->m_Name = j["name"];
            scene->m_Path = filePath;
            scene->m_PlayerStartLocation = StringToPoint (j["player_start"]);

            std::vector<int> ints = StringToInts (j["color"]);
            scene->m_BackgroundColor = al_map_rgba (ints[0], ints[1], ints[2], ints[3]);

            scene->m_Size = Rect (StringToPoint (j["min_size"]), StringToPoint (j["max_size"]));
            scene->m_Size = Rect (Point (-1000, -1000), Point (1000, 1000));

            scene->m_QuadTree = QuadTreeNode (scene->m_Size);

            auto& actors = j["actors"];

            for (auto& actorIt : actors)
            {
                Actor* newActor = ActorFactory::GetActor (sceneManager, actorIt["type"]);

                if (newActor)
                {
                    std::string id = actorIt["id"];
                    newActor->ID = atoi (id.c_str ());
                    newActor->Name = actorIt["name"];
                    newActor->Bounds.Pos = StringToPoint (actorIt["pos"]);
                    newActor->Bounds.Size = StringToPoint (actorIt["size"]);

                    std::string zOrder = actorIt["z-order"];
                    newActor->ZOrder = atoi (zOrder.c_str ());
                    std::string rot = actorIt["rot"];
                    newActor->Rotation = atof (rot.c_str ());

                    if (newActor->GetTypeName () == TileActor::TypeName)
                    {
                        dynamic_cast<TileActor*> (newActor)->Tileset = actorIt["tileset"];
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

                        Script* script = sceneManager->GetMainLoop ()->GetScriptManager ().LoadScriptFromFile (
                            GetDataPath () + "scripts/" + path, name + "_" + id);

                        if (script)
                        {
                            newActor->AttachScript (script, path);
                        }
                    }

                    newActor->SetCollisionEnabled (false);
                    newActor->Initialize ();
                    newActor->TemplateBounds = newActor->Bounds;

                    if (newActor->GetTypeName () == TileActor::TypeName)
                    {
                        std::string name = actorIt["name"];
                        name += "_";
                        name += actorIt["id"];
                        scene->AddTile (dynamic_cast<TileActor*> (newActor));
                    }
                    else
                    {
                        scene->AddActor (actorIt["name"], newActor);
                    }
                }
            }

            UpdateMaxEntityID (scene);
            scene->SortActors ();

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
                                for (int i = 0; i < connections.size (); ++i)
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
                std::vector<Point> poly = StringToVectorPoints (triggerArea["poly"]);

                scene->AddTriggerArea (name, poly);
            }

            auto& speeches = j["speeches"];

            for (auto& speech : speeches)
            {
                SpeechData speechData;
                speechData.Name = speech["name"];
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

                Script* script = sceneManager->GetMainLoop ()->GetScriptManager ().LoadScriptFromFile (
                    GetDataPath () + "scripts/" + path, name);

                if (script)
                {
                    scene->AttachScript (script, path);
                }
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
        Point minRect{ std::numeric_limits<float>::max (), std::numeric_limits<float>::max () };
        Point maxRect{ std::numeric_limits<float>::min (), std::numeric_limits<float>::min () };

        try
        {
            json j;

            j["name"] = scene->m_Name;

            ALLEGRO_COLOR color = scene->m_BackgroundColor;
            j["color"] = IntsToString (
                { (int)(color.r * 255.f), (int)(color.g * 255.f), (int)(color.b * 255.f), (int)(color.a * 255.f) });

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

                actorObj["id"] = ToString (actor->ID);
                actorObj["type"] = actor->GetTypeName ();
                actorObj["name"] = actor->Name;
                actorObj["pos"] = PointToString (actor->Bounds.GetPos ());
                actorObj["size"] = PointToString (actor->Bounds.GetSize ());
                actorObj["z-order"] = ToString (actor->ZOrder);
                actorObj["rot"] = ToString (actor->Rotation);

                if (actor->GetTypeName () == TileActor::TypeName)
                {
                    actorObj["tileset"] = dynamic_cast<TileActor*> (actor)->Tileset;
                }

                actorObj["phys"] = json::array ({});

                for (int i = 0; i < actor->PhysPoints.size (); ++i)
                {
                    json physObj = json::object ({});

                    physObj["poly"] = VectorPointsToString (actor->PhysPoints[i]);

                    actorObj["phys"].push_back (physObj);
                }

                actorObj["scripts"] = json::array ({});

                for (ScriptMetaData& script : actor->GetScripts ())
                {
                    json scriptObj = json::object ({});

                    scriptObj["name"] = script.Name;
                    scriptObj["path"] = script.Path;

                    actorObj["scripts"].push_back (scriptObj);
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
                for (int i = 0; i < it->second.Connections.size (); ++i)
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
                triggerObj["poly"] = VectorPointsToString (it->second.Points);

                j["trigger_areas"].push_back (triggerObj);
            }

            j["speeches"] = json::array ({});

            for (std::map<std::string, SpeechData>::iterator it = scene->m_Speeches.begin ();
                 it != scene->m_Speeches.end (); ++it)
            {
                json speechObj = json::object ({});

                speechObj["name"] = it->first;
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

                for (int i = 0; i < it->second.Outcomes[LANG_EN].size (); ++i)
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

                for (int i = 0; i < it->second.Outcomes[LANG_PL].size (); ++i)
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
