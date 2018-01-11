// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "SceneLoader.h"
#include "ActorFactory.h"
#include "Atlas.h"
#include "AtlasManager.h"
#include "MainLoop.h"
#include "SceneManager.h"

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
            std::ifstream file (filePath.c_str ());
            json j;
            file >> j;
            file.close ();

            scene->m_Name = j["name"];
            scene->m_Size = Rect (StringToPoint (j["min_size"]), StringToPoint (j["max_size"]));
            scene->m_Size = Rect (Point (-1000, -1000), Point (1000, 1000));

            scene->m_QuadTree = QuadTreeNode (scene->m_Size);

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

            auto& tiles = j["tiles"];

            for (auto& j_tile : tiles)
            {
                Tile* tile = new Tile (&sceneManager->GetMainLoop ()->GetPhysicsManager ());

                std::string id = j_tile["id"];
                tile->ID = atoi (id.c_str ());
                tile->Tileset = j_tile["tileset"];
                tile->Name = j_tile["name"];
                tile->Bounds.SetPos (StringToPoint (j_tile["pos"]));
                tile->Bounds.SetSize (
                  sceneManager->GetAtlasManager ()->GetAtlas (tile->Tileset)->GetRegion (tile->Name).Bounds.GetSize ());
                std::string zOrder = j_tile["z-order"];
                tile->ZOrder = atoi (zOrder.c_str ());
                std::string rot = j_tile["rot"];
                tile->Rotation = atof (rot.c_str ());

                //  Physics
                if (!j_tile["phys"].is_null ())
                {
                    auto& physTiles = j_tile["phys"];

                    for (auto& physTile : physTiles)
                    {
                        tile->PhysPoints.push_back (StringToVectorPoints (physTile["poly"]));
                    }

                    tile->SetPhysOffset (tile->Bounds.GetPos ());
                }

                scene->AddTile (tile);
            }

            auto& flag_points = j["flag_points"];

            for (auto& flag_point : flag_points)
            {
                std::string name = flag_point["name"];
                Point pos = StringToPoint (flag_point["pos"]);

                scene->AddFlagPoint (name, pos);
            }

            for (std::map<std::string, FlagPoint>::iterator it = scene->m_FlagPoints.begin ();
                 it != scene->m_FlagPoints.end ();
                 ++it)
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
                                 it2 != scene->m_FlagPoints.end ();
                                 ++it2)
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

                scene->AddSpeech (speechData.Name, speechData);
            }

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
                    newActor->TemplateBounds = newActor->Bounds;
                    std::string zOrder = actorIt["z-order"];
                    newActor->ZOrder = atoi (zOrder.c_str ());
                    std::string rot = actorIt["rot"];
                    newActor->Rotation = atof (rot.c_str ());

                    scene->AddActor (actorIt["name"], newActor);
                }
            }

            UpdateMaxEntityID (scene);
            scene->SortTiles ();
            scene->SortActors ();

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

            j["scripts"] = json::array ({});

            for (std::vector<ScriptMetaData>::iterator it = scene->m_Scripts.begin (); it != scene->m_Scripts.end ();
                 ++it)
            {
                json scriptObj = json::object ({});

                scriptObj["name"] = it->Name;
                scriptObj["path"] = it->Path;

                j["scripts"].push_back (scriptObj);
            }

            j["tiles"] = json::array ({});

            for (Tile* tile : scene->m_Tiles)
            {
                if (tile->Bounds.GetTopLeft ().X < minRect.X)
                {
                    minRect.X = tile->Bounds.GetTopLeft ().X;
                }
                if (tile->Bounds.GetTopLeft ().Y < minRect.Y)
                {
                    minRect.Y = tile->Bounds.GetTopLeft ().Y;
                }
                if (tile->Bounds.GetBottomRight ().X > maxRect.X)
                {
                    maxRect.X = tile->Bounds.GetBottomRight ().X;
                }
                if (tile->Bounds.GetBottomRight ().Y > maxRect.Y)
                {
                    maxRect.Y = tile->Bounds.GetBottomRight ().Y;
                }

                json tileObj = json::object ({});

                tileObj["id"] = ToString (tile->ID);
                tileObj["tileset"] = tile->Tileset;
                tileObj["name"] = tile->Name;
                tileObj["pos"] = PointToString (tile->Bounds.GetPos ());
                tileObj["z-order"] = ToString (tile->ZOrder);
                tileObj["rot"] = ToString (tile->Rotation);

                tileObj["phys"] = json::array ({});

                for (int i = 0; i < tile->PhysPoints.size (); ++i)
                {
                    json physObj = json::object ({});

                    physObj["poly"] = VectorPointsToString (tile->PhysPoints[i]);

                    tileObj["phys"].push_back (physObj);
                }

                j["tiles"].push_back (tileObj);
            }

            j["min_size"] = PointToString (minRect);
            j["max_size"] = PointToString (maxRect);

            j["flag_points"] = json::array ({});

            for (std::map<std::string, FlagPoint>::iterator it = scene->m_FlagPoints.begin ();
                 it != scene->m_FlagPoints.end ();
                 ++it)
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
                 it != scene->m_TriggerAreas.end ();
                 ++it)
            {
                json triggerObj = json::object ({});

                triggerObj["name"] = it->second.Name;
                triggerObj["poly"] = VectorPointsToString (it->second.Points);

                j["trigger_areas"].push_back (triggerObj);
            }

            j["speeches"] = json::array ({});

            for (std::map<std::string, SpeechData>::iterator it = scene->m_Speeches.begin ();
                 it != scene->m_Speeches.end ();
                 ++it)
            {
                json speechObj = json::object ({});

                speechObj["name"] = it->first;
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

            j["actors"] = json::array ({});

            for (Actor* actor : scene->m_Actors)
            {
                json actorObj = json::object ({});

                actorObj["id"] = ToString (actor->ID);
                actorObj["type"] = actor->GetTypeName ();
                actorObj["name"] = actor->Name;
                actorObj["pos"] = PointToString (actor->Bounds.GetPos ());
                actorObj["z-order"] = ToString (actor->ZOrder);
                actorObj["rot"] = ToString (actor->Rotation);

                j["actors"].push_back (actorObj);
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
        std::vector<Tile*>& tiles = scene->GetTiles ();
        int maxEntID = -1;

        for (Tile* t : tiles)
        {
            if (t->ID > maxEntID)
            {
                maxEntID = t->ID + 1;
            }
        }

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
